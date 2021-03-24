from global_vars import *
from package import *
from util import *
from opts import *


help_ = """                  _           _        _ _
  __ _ _   _ _ __(_)_ __  ___| |_ __ _| | |
 / _` | | | | '__| | '_ \/ __| __/ _` | | |
| (_| | |_| | |  | | | | \__ \ || (_| | | |
 \__,_|\__,_|_|  |_|_| |_|___/\__\__,_|_|_|

Simple AUR helper written in Python.

Valid operations are install, search, update, clean, and remove.

 - install multiple packages with one command
 - specify more than one search term to refine your search

Options:
 --noconfirm - automatically confirm all transactions

Config:
 ~/.config/aurinstall/aurinstall

 blacklist: pkg1 pkg2 pkg3
    - blacklists listed packages
 autoupdate
    - enables updating aurinstall from GitHub when running "aurinstall update".

- written by Hasan Zahra
https://github.com/HasanQZ/aurinstall"""

if __name__ == '__main__':
    parse_opt_args()
    parse_config_file()
    onstart_log()

    argc = len(argv)

    if argc == 1:
        print(help_)
        exit()

    if argc == 2:
        if argv[1] not in ['install', 'search', 'remove', 'update', 'clean']:
            print(f'Invalid operation: {argv[1]}')
        elif argv[1] == 'update':
            update()
            if opts['autoupdate']:
                print('warning: updating aurinstall from GitHub enabled.')
                update_script()
        elif argv[1] == 'clean':
            clean()
        else:
            print(f'Please provide an argument for operation: {argv[1]}')

    elif argc >= 3:
        if argv[1] == 'clean':
            clean()
        if argv[1] == 'install':
            install_packages(argv[2:])

        elif argv[1] == 'search':
            search_package(argv[2:])

        elif argv[1] == 'remove':
            remove_packages(argv[2:])
    else:
        print(f'invalid arguments: {argv}')
