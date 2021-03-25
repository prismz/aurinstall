from global_vars import *
from util import *
import sys

# sets flags read from optional arguments (--noconfirm, etc) and returns all non-optional arguments
def parse_opt_args():
    for arg in sys.argv:
        if arg.strip().startswith('-'):
            optargv.append(arg.lower().strip())
        else:
            argv.append(arg.strip())

    argc = len(argv)

    for arg in optargv:
        if arg == '--noconfirm':
            opts['noconfirm'] = True
            opts['pacman_args'] += ' --noconfirm'

        elif arg.startswith('--color='):
            colorflag = arg.split('=', 1)[0].lower().strip()
            if colorflag == 'auto' or colorflag == 'smart':
                opts['color'] = SMART
                opts['pacman_args'] += ' --color auto '
            elif colorflag == 'yes':
                opts['color'] = True
                opts['pacman_args'] += ' --color always '
            elif colorflag == 'no':
                opts['pacman_args'] += ' --color never '
                opts['color'] = False

        elif arg.startswith('--verbosity='):
            verbosity_level = int(arg.split('=', 1)[0])
            if verbosity_level > MAX_LOGLEVEL:
                verbosity_level = MAX_LOGLEVEL

            opts['verbosity'] = verbosity_level

        elif arg == '--quiet':
            opts['quiet'] = True
            opts['git_args'] += ' --quiet '
            opts['pacman_args'] += ' --quiet '

        else:
            print_err(f'argument {arg} not recognized.')

    opts['args_parsed'] = True

def parse_config_file():
    config_file = open(config_path, 'r+')
    config = config_file.read()
    config_file.close()

    for _line in config.split('\n'):
        line = _line.strip().lower()
        try:
            if line == 'autoupdate':
                opts['autoupdate'] = True
            
            elif line.startswith('blacklist'):
                pkgs = line.split(' ')[1:]
                opts['blacklist'] += pkgs

            elif line.startswith('onupdate_command'):
                command = line.split(' ', 1)[-1]
                opts['onupdate_command'] = command
            
            elif line.startswith('pacman_args'):
                args = line.split(' ', 1)[-1]
                opts['pacman_args'] += f'  {args}  '
            
            elif line.startswith('git_args'):
                args = line.split(' ', 1)[-1]
                opts['git_args'] += f' {args} '
            elif line == '':
                continue
            else:
                print_err(f'unrecognized line in config: {_line}.')

        except:
            print_err(f'error parsing line in config: {_line}.')
    
    opts['config_parsed'] = True
    


