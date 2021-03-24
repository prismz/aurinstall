import subprocess
import requests
import sys
import os

from util import *
from global_vars import *

def print_package_info(metadata):
    version = metadata['Version']
    name = metadata['Name']
    if normal_term and opts['coloroutput'] in (SMART, True):
        print(f'{BOLD}{RED}aur/{ENDC}{ENDC}' + f'{BOLD}{name}{ENDC} ' + f'{BOLD}{GREEN}{version}{ENDC}{ENDC}')
        pretty_print(metadata['Description'])
    elif not (normal_term and opts['coloroutput'] not in (SMART, True)):
        print(f'aur/{name} {version}')
        print('    ',end='')
        print(metadata['Description'])
    else:
        print(f'error: target not found: {name}')

def install_packages(packages):
    non_aur_package_str = ''
    api_str = f'https://aur.archlinux.org/rpc/?v=5&type=info'
    pacopts = opts['pacman_args']
    gitopts = opts['git_args']

    for package in packages:
        api_str += f'&arg[]={package}'

    metadata = requests.get(api_str).json()
    result_count = metadata['resultcount']

    aur_packages = {}
    for package in metadata['results']:
        aur_packages[package['Name']] = package

    if result_count != len(packages):
        for package in packages:
            if package not in aur_packages:
                non_aur_package_str += package + ' '

        retc = os.system(f'sudo pacman {pacopts} -S {non_aur_package_str}')
        if retc != 0:
            print('error installing non-AUR packages.')

    if aur_packages == {}:
        return

    for pkgname in aur_packages:
        pkgdata = aur_packages[pkgname]

        name = pkgdata['Name']

        if pkgdata['OutOfDate'] not in [None, 'null']:
            prompt = input(f'warning: package {name} is out of date. Continue? [y/N] ')
            if prompt.lower() != 'y':
                continue  # skip current package

        package_path = f'{cache_path}/{name}/'
        clone_success = os.system(f'git clone https://aur.archlinux.org/{name}.git {package_path}')

        if (clone_success and os.path.exists(package_path)):
            cleanbuild = input(f'files already exist for package {name}. Rebuild package? [y/N] ')
            if cleanbuild.lower().strip() == 'y':
                os.system(f'rm -rf {package_path}')
                result_ = os.system(f'git clone https://aur.archlinux.org/{name}.git {package_path}')
                if result_:
                    print(f'error: error installing package {name}')
                    continue
        
        retc = os.system(f'cd {package_path} && pwd && makepkg {pacopts} -si {package_path}/')

        if retc != 0:
            print('warning: non-zero return code from package build.')

def clean():
    packages_in_cache = os.listdir(cache_path)
    if len(packages_in_cache) == 0:
        print('no packages in cache.')
        return

    package_dict = {}

    for i, pkg in enumerate(packages_in_cache):
        print(f'{i+1}. {pkg}')
        package_dict[i+1] = pkg

    x = input(f'select package caches to clean (leave blank for all, -1 for none): ')
    if x.strip() == '':
        for pkg in packages_in_cache:
            r = os.system(f'rm -rf {cache_path}/{pkg}/')
            if r != 0:
                print(f'error cleaning cache of package {pkg}')
            else:
                print(f'cleaned cache: {cache_path}/{pkg}')
    elif x.strip() == '-1':
        return

    else:
        for ind in x.strip().split(' '):
            try:
                pkg = package_dict[int(ind)]
                r = os.system(f'rm -rf {cache_path}/{pkg}/')
                if r != 0:
                    print(f'error cleaning cache of package {pkg}')

                else:
                    print(f'cleaned cache {cache_path}/{pkg}')
            except:
                print(f'invalid package index: {ind}')

def update_script():
    print('cloning aurinstall from github to ensure latest version...')
    os.system(f'rm -rf {cache_path}/aurinstall/')
    clone_failed = os.system(f'git clone --quiet https://github.com/hasanqz/aurinstall {cache_path}/aurinstall/ >> /dev/null')
    if clone_failed:
        print('error: error cloning new aurinstall version')
        return

    os.system(f'sudo cp {cache_path}/aurinstall/aurinstall /usr/bin/aurinstall && sudo chmod +x /usr/bin/aurinstall')
    print('updated to latest aurinstall version.')

def update():
    gitopts = opts['git_args']
    pacopts = opts['pacman_args']

    print('updating standard packages...')

    os.system(f'sudo pacman {pacman_args} -Syu')
    print('\nchecking AUR packages for updates...')
    aur_pkgs = [pkg for pkg in subprocess.getoutput('pacman -Qm').split('\n') if pkg.split(' ', 1)[0] not in opt_blacklist]

    api_str = f'https://aur.archlinux.org/rpc/?v=5&type=info'
    pkgs = {}

    to_update = []

    if opt_blacklist != []:
        print(f'Packages have been blacklisted:')
        for pkg in opt_blacklist:
            print(f' => {pkg}')

    # pkg_list = subprocess.getoutput('pacman -Qm').split('\n')
    for pkg in aur_pkgs:
        pkg_name, pkg_ver = pkg.split(' ', 1)
        api_str += f'&arg[]={pkg_name}'
        pkgs[pkg_name] = pkg_ver

    metadata = requests.get(api_str).json()
    if metadata['resultcount'] <= 0:
        if aur_pkgs == []:
            return
        else:
            for pkg in aur_pkgs:
                print(f'package {pkg} is invalid or not an AUR package.')
            return

    for result in metadata['results']:
        name = result['Name']
        ver = result['Version']
        ood = result['OutOfDate']

        if ood not in (None, 'null'):
            opt = input(f'package {name} has been flagged out of date. continue? [y/N] ')
            if opt.lower().strip() != 'y':
                continue

        if pkgs[name] != ver:
            os.system(f'rm -rf {cache_path}/{name}')
            to_update.append(name)

    if to_update != []:
        install_packages(to_update)
    else:
        print('no AUR packages to update!')

def remove_packages(packages):
    pstr = ''.join([i + ' ' for i in packages])
    os.system(f'sudo pacman {pacargs} -R {pstr}')

def search_package(terms):
    pstr = ''.join([i + ' ' for i in terms])
    rc = os.system(f'pacman {pacargs} -Ss {pstr}')

    api_str = f'https://aur.archlinux.org/rpc/?v=5&type=search&arg={terms[0]}'

    json = requests.get(api_str).json()

    if json['resultcount'] == 0 and rc:
        print('error: no packages found.')
        return

    rw_package_data = json['results']
    package_data = {}
    for i in rw_package_data:
        package_data[i['Name']] = i

    packages_to_show = []

    for rsp in package_data:
        dt = package_data[rsp]
        name = dt['Name']
        desc = dt['Description']

        if all_list_in_str(f'{name} {desc}', terms[1:]):
            packages_to_show.append(dt)

    for i in packages_to_show:
        print_package_info(i)
