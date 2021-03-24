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
        elif arg.startswith('--color='):
            colorflag = arg.split('=', 1)[0].lower().strip()
            if colorflag == 'auto' or colorflag == 'smart':
                opts['color'] = SMART
                opts['pacman_args'] += ' --color auto'
            elif colorflag == 'yes':
                opts['color'] = True
                opts['pacman_args'] += ' --color always'
            elif colorflag == 'no':
                opts['pacman_args'] += ' --color never'
                opts['color'] = False
        elif arg.startswith('--verbosity='):
            verbosity_level = int(arg.split('=', 1)[0])
            if verbosity_level > MAX_LOGLEVEL:
                verbosity_level = MAX_LOGLEVEL

            opts['verbosity'] = verbosity_level

        else:
            print_err(f'argument {arg} not recognized.')

    opts['args_parsed'] = True

