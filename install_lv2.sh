#!/bin/sh
#
# Install lv2 plugin bundle
#

if test -z "${LV2_INSTALL_PATH}" -a "${LV2_PATH}"
then
  echo "Trying to deduce install location from LV2_PATH contents..."
  OLDIFS=${IFS}
  IFS=:
  for DEST in ${LV2_PATH}
  do
    if test -d "${DEST}" -a -w "${DEST}"
    then
      echo "${DEST} looks good."
      LV2_INSTALL_PATH=${DEST}
      break
    else
      echo "${DEST} is not writable directory..."
    fi
  done
  #echo "${LV2_PATH} is directory. Installing there..."
  #LV2_INSTALL_PATH=${LV2_PATH}
  IFS=${OLDIFS}
fi

if test -z "${LV2_INSTALL_PATH}"
then
  echo "Please specify where to install by supplying LV2_INSTALL_PATH"
  exit 1
fi

echo "Installing ${1} to ${LV2_INSTALL_PATH}"
cp -R ${1} ${LV2_INSTALL_PATH}

exit $?
