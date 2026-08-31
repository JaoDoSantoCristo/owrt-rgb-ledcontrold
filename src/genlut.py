# OpenWrt RGB LED control daemon
# Copyright (C) 2026  Jão do Santo Cristo
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

def gamma_correct(linear):
  gamma_val = linear ** (1 / 2.2) 
  u8_val = round(gamma_val * 255)
  return u8_val

nums = []
elems = 256
for i in range(elems):
  nums.append(gamma_correct( float(float(i) / float(elems)) ))

print(nums)
