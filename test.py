
import sys
import os

from gimli import SSHClient, printc, GREEN, __version__

'''
try:
    host = sys.argv[1]
except IndexError as e:
    sys.exit("Usage: python3 {}".format(__file__))
'''

print(__version__)

ssh = SSHClient()

_, output = ssh.run_command('ls')
printc(output, GREEN, end='-----\n')
printc(output, end='')
printc('hello!')
# printc('world', BLUE)

print(ssh.can_ping('google.com'))

ssh.ssh_exec('192.168.1.17', 'ls')
