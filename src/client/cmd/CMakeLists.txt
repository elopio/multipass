# Copyright © 2017 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>

add_library(commands STATIC
  animated_spinner.cpp
  connect.cpp
  create.cpp
  empty_trash.cpp
  exec.cpp
  find.cpp
  help.cpp
  info.cpp
  list.cpp
  mount.cpp
  recover.cpp
  start.cpp
  stop.cpp
  trash.cpp
  umount.cpp
  version.cpp

  ${PLATFORM_COMMANDS})

target_link_libraries(commands
  ssh_client
  rpc
  Qt5::Core
  yaml)
