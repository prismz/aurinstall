import os
import subprocess

SMART = 3   # sort of like a 3rd boolean option, for "smart" detection of environment variables.
MAX_LOGLEVEL = 3

BOLD = '\033[1m'
GREEN = '\033[92m'
RED = '\033[91m'
ENDC = '\033[0m'
ROOT_DIR = os.path.dirname(__file__)

home_folder = os.path.expanduser('~')
application_log = {}  # key:log contents, value=verbosity level required to show this log


normal_term = True
try:
    os.get_terminal_size()
except OSError:
    normal_term = False

cache_path = f'{home_folder}/.cache/aurinstall'
config_path = f'{home_folder}/.config/aurinstall/aurinstall'

if not os.path.exists(cache_path):
    os.system(f'mkdir {cache_path}')
if not os.path.exists(f'{home_folder}/.config/aurinstall'):
    os.system(f'mkdir {home_folder}/.config/aurinstall')
os.system(f'touch {config_path}')

optargv = []
argv = []

opts = {
    'noconfirm': False,
    'autoupdate': False,
    'coloroutput': SMART,    # smart=detect terminal, otherwise boolean value
    'quiet': False,          # if True, don't print anything to command line
    'verbosity': 0,          # -1=no output at all(--quiet), 0=off, then 1-3 for verbosity level
    'args_parsed': False,    # whether or not arguments have been parsed yet
    'config_parsed': False,  # whether or not the config file has been parsed yet
    'pacman_args': '',       # string of arguments to pass to ALL pacman calls
    'git_args': '',          # string of arguments to pass to ALL git calls
    'blacklist': [],         # packages to blacklist (only works with AUR packages)
    'onupdate_command': ''    # command to run on update
}
