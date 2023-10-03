#!/bin/bash
set -eu

# Sourced from https://github.com/Aneurysm9/vm_challenge
md5hashes=(
    '76ec2408e8fe3f1753c25db51efd8eb3'
    '0e6aa7be1f68d930926d72b3741a145c'
    '7997a3b2941eab92c1c0345d5747b420'
    '186f842951c0dcfe8838af1e7222b7d4'
    '2bf84e54b95ce97aefd9fc920451fc45'
    'e09640936b3ef532b7b8e83ce8f125f4'
    '4873cf6b76f62ac7d5a53605b2535a0c'
    'd0c54d4ed7f943280ce3e19532dbb1a6'
)

validate() {
    idx=$1
    want=${md5hashes[$((idx - 1))]}
    code=$(echo -n $2 | md5sum | head -c 32)
    if [[ "${code}" == "${want}" ]]; then
        echo "Code #${idx} matches"
    else
        echo "Mismatching code #${idx}:"
        echo "    want: ${want}"
        echo "     got: ${code}"
        exit 1
    fi
}

tmp=$(mktemp)

cin="take tablet
use tablet
doorway
north
north
bridge
continue
down
east
take empty lantern
west
west
passage
ladder
west
south
north
take can
look can
use can
west
ladder
darkness
use lantern
continue
west
west
west
west
north
take red coin
north
west
take blue coin
up
take shiny coin
down
east
east
take concave coin
down
take corroded coin
up
west
use blue coin
use red coin
use shiny coin
use concave coin
use corroded coin
north
take teleporter
use teleporter
"

( ./build/Release/vm/cmd/runvm docs/spec/challenge <<< ${cin} ) | tee "$tmp" &

# Keep it running for 5 seconds, then kill the process
sleep 1
pid=$(ps -C 'runvm' -o pid --no-headers)
[ -z "${pid}" ] || {
    echo "---"
    echo "Killing process"
    kill ${pid}
    echo "Process killed"
}

echo "---"

code1=`grep "Here's a code for the challenge website:" "docs/spec/spec.txt" | sed 's#^.*: \(.*\)$#\1#'`
validate 1 $code1

code2=`grep 'this one into the challenge website:' "$tmp" | sed 's#^.*: \(.*\)$#\1#'`
validate 2 "${code2}"

code3=`grep 'The self-test completion code is:' "$tmp" | sed 's#^.*: \(.*\)$#\1#'`
validate 3 "${code3}"


code4=`grep " on the tablet.  Perhaps it's some kind of code?" "$tmp" | sed 's#^[^"]\+"\(\w\+\)".*$#\1#'`
validate 4 "${code4}"

codes56=(`grep '^    [a-zA-Z]\{12\}$'  "$tmp"`)
validate 5 "${codes56[0]}"
validate 6 "${codes56[1]}"

code7="No idea"
validate 7 "${code7}"