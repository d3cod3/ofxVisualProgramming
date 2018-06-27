#!/usr/bin/env bash
set -e

OF_CLONE_BRANCH="TEMP-CI"
OF_CLONE_USERNAME=d3cod3

DEFAULT_CLONE_DEPTH=1

if [ "$CI" = true ];
then
  OF_ROOT=${OF_ROOT:-~/openFrameworks}

  THIS_ADDON_NAME=${TRAVIS_REPO_SLUG#*/}
  THIS_USERNAME=${TRAVIS_REPO_SLUG%/*}
  THIS_BRANCH=${TRAVIS_BRANCH}

else
  OF_ROOT=${OF_ROOT:-$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../.." && pwd )}
  OF_ADDONS_DIR=${OF_ADDONS_DIR:-${OF_ROOT}/addons}

  THIS_ADDON_NAME=$(basename $( cd "$( dirname "${BASH_SOURCE[0]}")/../.."  && pwd ))
  THIS_USERNAME=$(whoami)
  THIS_BRANCH=$(git rev-parse --abbrev-ref HEAD)
fi

OF_ADDONS_DIR=${OF_ADDONS_DIR:-${OF_ROOT}/addons}
THIS_ADDON_DIR=${OF_ADDONS_DIR}/${THIS_ADDON_NAME}

OF_CLONE_DEPTH=${OF_CLONE_DEPTH:-${DEFAULT_CLONE_DEPTH}}
OF_CLONE_BRANCH=${OF_CLONE_BRANCH:-${THIS_BRANCH}}
OF_CLONE_USERNAME=${OF_CLONE_USERNAME:-openFrameworks}

ADDON_CLONE_DEPTH=${ADDON_CLONE_DEPTH:-${DEFAULT_CLONE_DEPTH}}
ADDON_CLONE_BRANCH=${ADDON_CLONE_BRANCH:-${THIS_BRANCH}}
ADDON_CLONE_USERNAME=${ADDON_CLONE_USERNAME:-${THIS_USERNAME}}


echo "  THIS_USERNAME: ${THIS_USERNAME}"
echo "    THIS_BRANCH: ${THIS_BRANCH}"
echo ""
echo "      OF_CLONE_DEPTH: ${OF_CLONE_DEPTH}"
echo "     OF_CLONE_BRANCH: ${OF_CLONE_BRANCH}"
echo "   OF_CLONE_USERNAME: ${OF_CLONE_USERNAME}"
echo ""
echo "   ADDON_CLONE_DEPTH: ${ADDON_CLONE_DEPTH}"
echo "  ADDON_CLONE_BRANCH: ${ADDON_CLONE_BRANCH}"
echo "ADDON_CLONE_USERNAME: ${ADDON_CLONE_USERNAME}"
echo ""
echo "        OF_ROOT: ${OF_ROOT}"
echo "  OF_ADDONS_DIR: ${OF_ADDONS_DIR}"
echo "THIS_ADDON_NAME: ${THIS_ADDON_NAME}"
echo " THIS_ADDON_DIR: ${THIS_ADDON_DIR}"

echo "---------------------"
bash --version
echo "---------------------"

# Takes a string and removes all duplicate tokens.
function sort_and_remove_duplicates()
{
    echo $(echo ${1} | tr ' ' '\n' | sort -u | tr '\n' ' ')
    return 1
}

# Extract ADDON_DEPENDENCIES from an addon's addon_config.mk file.
function get_dependencies_for_addon()
{
  if [ -z "$1" ]; then
    echo "Usage: get_dependencies_for_addon <path_to_addon>"
    return 1
  fi
  if [ -f ${1}/addon_config.mk ]; then
    local ADDON_DEPENDENCIES=""
    while read line; do
      if [[ $line == ADDON_DEPENDENCIES* ]] ;
      then
        line=${line#*=}
        IFS=' ' read -ra ADDR <<< "$line"
        for i in "${ADDR[@]}"; do
          ADDON_DEPENDENCIES="${ADDON_DEPENDENCIES} ${i}"
        done
      fi
    done < ${1}/addon_config.mk
    echo $(sort_and_remove_duplicates "${ADDON_DEPENDENCIES}")
  fi
  return 0
}

# Extract ADDON_DEPENDENCIES from an addon's example addons.make files.
function get_dependencies_for_addon_examples()
{
  if [ -z "$1" ]; then
    echo "Usage: get_dependencies_for_addon_examples <path_to_addon>"
    return 1
  fi

  local ADDONS_REQUIRED_BY_EXAMPLES=""

  for addons_make in ${1}/example*/addons.make; do
    while read addon; do
      ADDONS_REQUIRED_BY_EXAMPLES="${ADDONS_REQUIRED_BY_EXAMPLES} ${addon}"
    done < ${addons_make}
  done
  echo $(sort_and_remove_duplicates "${ADDONS_REQUIRED_BY_EXAMPLES}")
  return 0
}


function get_all_dependencies_for_addon()
{
  if [ -z "$1" ]; then
    echo "Usage: get_all_dependencies_for_addon <path_to_addon>"
    return 1
  fi

  local ADDONS_REQUIRED=$(get_dependencies_for_addon "$1")
  local ADDONS_REQUIRED_BY_EXAMPLES=$(get_dependencies_for_addon_examples "$1")

  echo $(sort_and_remove_duplicates "${ADDONS_REQUIRED} ${ADDONS_REQUIRED_BY_EXAMPLES}")
  return 0

}

# Clone the list of addons and check to make sure all dependencies are satisfied and cloned.
function clone_addons()
{
  if [ -z "$1" ]; then
    echo "Usage: List of addons to download."
    return 1
  fi

  for addon in "$@"
  do
    if [ ! -d ${OF_ADDONS_DIR}/${addon} ]; then
      echo "Installing: ${OF_ADDONS_DIR}/${addon}"

      if [ ${ADDON_CLONE_USERNAME} = "ofxLua" ]
      then
        git clone --quiet --depth=${ADDON_CLONE_DEPTH} --branch=of-0.10.0 https://github.com/${ADDON_CLONE_USERNAME}/${addon}.git ${OF_ADDONS_DIR}/${addon}
      else
        git clone --quiet --depth=${ADDON_CLONE_DEPTH} -b ${ADDON_CLONE_BRANCH} https://github.com/${ADDON_CLONE_USERNAME}/${addon}.git ${OF_ADDONS_DIR}/${addon}
      fi



      local _REQUIRED_ADDONS=$(get_dependencies_for_addon ${OF_ADDONS_DIR}/${addon})

      for required_addon in ${_REQUIRED_ADDONS}
      do
        if [ ! -d ${OF_ADDONS_DIR}/${required_addon} ]; then
          clone_addons ${required_addon}
        else
          echo "Dependency satisfied: ${required_addon}"
        fi
      done
    fi
  done
  return 0
}


# Make it easier to read output from make.
export MAKEFLAGS=-s


function do_install {
    cd ~
    git clone --depth=${OF_CLONE_DEPTH} --branch=${OF_CLONE_BRANCH} https://github.com/${OF_CLONE_USERNAME}/openFrameworks
    cd openFrameworks
    scripts/ci/addons/install.sh
    clone_addons $(get_all_dependencies_for_addon ${THIS_ADDON_DIR})
}


function do_script {
    cd ~
    cd openFrameworks
    scripts/ci/addons/build.sh
}
