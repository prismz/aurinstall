import os
import subprocess

SMART = 3
MAX_LOGLEVEL = 3

home_folder = os.path.expanduser('~')
application_log = {} # key:log contents, value=verbosity level required to show this log

normal_term = True
try:
    os.get_terminal_size()
except OSError:
    normal_term = False

cache_path = f'{home_folder}/.cache/aurinstall'
config_path = f'{home_folder}/.config/aurinstall/aurinstall'

optargv = []
argv = []

opts = {
    'noconfirm': False,
    'autoupdate': False,
    'coloroutput': SMART,    # smart=detect terminal, otherwise boolean value
    'quiet': False,          # if True, don't print anything to command line
    'verbosity': 0,          # -1=no output at all(--quiet), 0=off, then 1-3 for verbosity level
    'args_parsed': False,    # whether or not arguments have been parsed yet
    'pacman_args':''         # string of arguments to pass to ALL pacman calls
    'blacklist': [],         # packages to blacklist (only works with AUR packages)
}

