import os
import sys

os.chdir('Empire')
if sys.platform == 'win32':
  os.system('empire.exe')
else:
  os.system('./empire')
os.chdir('..')
