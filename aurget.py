#!/usr/bin/python

import requests
import os
import sys

BOLD = '\033[1m'
GREEN = '\033[92m'
RED = '\033[91m'
ENDC = '\033[0m'


home_folder = os.path.expanduser('~')

def pretty_print(text, indent=4, first_indent=4):
    termwidth = os.get_terminal_size().columns - indent
    words = [i for i in text.split(' ')]

    print(' ' * first_indent, end='')
    i = 0
    for index, word in enumerate(words):
        print(word, end=' ')
        i += len(word) + 1

        try:
            if i + (len(words[index + 1]) + 1) >= termwidth:
                print('')
                print(' ' * indent, end='')
                i = 0
        except IndexError:
            pass

    print('')

class Package:
    def __init__(self, name, metadata={}):
        self.valid_package = True
        self.name = name

        if metadata == {}:
            self.metadata = self.get_metadata()
        else:
            self.metadata = metadata
        
        
        
        self.clone_url = self.get_clone_url()


    def get_clone_url(self):
        return f'https://aur.archlinux.org/{self.name}.git'

    def get_metadata(self):
        package_query_str = f'https://aur.archlinux.org/rpc/?v=5&type=info&arg={self.name}'
        metadata = requests.get(package_query_str).json()
        if metadata['resultcount'] != 1:
            self.valid_package = False
        return metadata

    def print_info(self):
        if self.valid_package:
            version = self.metadata['Version']
            print(f'{BOLD}{RED}aur/{ENDC}{ENDC}' + f'{BOLD}{self.name}{ENDC} ' + f'{BOLD}{GREEN}{version}{ENDC}{ENDC}')
            pretty_print(self.metadata['Description'])
        else:
            print(f'error: target not found: {self.name}')

    def install(self):
        if not self.valid_package:
            print(f'error: target not found: {self.name}')
            return

        package_path = f'{home_folder}/.cache/aurget/{self.name}/'
        result = os.system(f'git clone {self.get_clone_url()} {package_path}')

        if result: # failure (non-zero return code)
            if os.path.exists(package_path): # if the package has already been cloned
                cleanbuild = input(f'Files already exist for package {self.name}. Rebuild package? [y/N] ')
                if cleanbuild.lower().strip() == 'y':
                    os.system(f'rm -rf {package_path}')
                    result_ = os.system(f'git clone {self.get_clone_url()} {package_path}')
                    if result_:
                        print(f'ERROR: error installing package {self.name}')
                        return


        os.system(f'cd {package_path} && pwd && makepkg -si {package_path}/')



def search_package(package_name, include_normal_packages=True):
    if include_normal_packages:
            os.system(f'pacman --color always -Ss {package_name}')

    package_query_str = f'https://aur.archlinux.org/rpc/?v=5&type=search&arg={package_name}'
    json_response = requests.get(package_query_str).json()

    if json_response['resultcount'] == 0:
        print('error: no packages found.')
        return

    for i in range(json_response['resultcount']):
        current_package_json_chunk = json_response['results'][i]
        current_package = Package(current_package_json_chunk['Name'], metadata=current_package_json_chunk)
        current_package.print_info()

def remove_package(package_name):
    os.system(f'sudo pacman -R {package_name}')

help_ = """help_ = AURget - A simple AUR helper written in Python.

Valid operations are install, search, and remove."""

def main():
    # I don't like argparse
    argc = len(sys.argv)
    argv = sys.argv

    if argc == 1:
        print(help_)

    if argc == 2:
        if argv[1] not in ['install', 'search', 'remove']:
            print(f'Invalid operation: {argv[1]}')
        else:
            print(f'Please provide an argument for operation: {argv[1]}')

    elif argc == 3:
        if argv[1] == 'install':
            package = Package(name=argv[2])
            package.install()

        elif argv[1] == 'search':
            aur_only = False
            if '--aur_only' in argv:
                aur_only = True

            search_package(argv[2], aur_only)

        elif argv[1] == 'remove':
            remove_package(argv[2])

if __name__ == '__main__':
    main()