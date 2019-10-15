# Copyright (C) 2019 Greenweaves Software Limited
#
# This is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this software.  If not, see <http://www.gnu.org/licenses/>
#

import glob, os.path, re

def find_seq(path='./imgs',seq=-1,prefix='energy',ext='png'):
    try:
        files   = sorted(glob.glob(os.path.join(path,'{0}*.{1}'.format(prefix,ext))))
        pattern = re.compile('.*{0}(\d+).{1}'.format(prefix,ext))
        nn      = int(re.match(pattern,files[seq]).group(1))
        return nn
    except IndexError:
        return -1
    