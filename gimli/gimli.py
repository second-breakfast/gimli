#
# gimli - And my axe!
#
# classes:
#   SSHClient - a minimalist SSH client
#
#   ...more coming soon!

from subprocess import check_output, CalledProcessError, STDOUT

__version__ = 'gimli 0.1'

class SSHClient():

    def __init__(self):
        pass

    def run_command(self, command):
        try:
            output = check_output(command, stderr=STDOUT, universal_newlines=True)
            t = 0, output
        except CalledProcessError as e:
            t = e.returncode, e.output
        return t

    def can_ping(self, host):
        status, output = self.run_command(['ping', '-c', '1', '-w', '1', host])
        return status == 0

    def ssh_exec(self, host, command):
        if self.can_ping(host):
            status, output = self.run_command(['ssh', '-i', '~/.ssh/root_key',
                                         '-oStrictHostKeyChecking=no',
                                         'root@'+host, command])
        else:
            status, output = 55555, "could not connect to " + host
        return status, output

    def scp(self, host, f, path=''):
        if self.can_ping(host):
            status, output = self.run_command(['scp', '-i', '~/.ssh/root_key',
                                         f, 'root@'+host+':'+path])
        else:
            status, output = 55555, "could not connect to " + host
        return status, output

