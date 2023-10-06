#!/bin/bash
set -eu

mkdir -p tmp
rm "tmp/*" 2>/dev/null || true

validate() {
    want='4873cf6b76f62ac7d5a53605b2535a0c'
    code=$(echo -n $2 | md5sum | head -c 32)
    if [[ "${code}" == "${want}" ]]; then
        echo "$1 -> $2 MATCH"
    else
        printf "$1 -> $2"'\r'
    fi
}

attempt () {
    solution="tmp/steps_${1}"
    cp solution.txt "${solution}"
    cat << EOF >> "${solution}"
!setr 7 ${1}
!abreak 0x1587
use teleporter
!peek
!abreak 0x1587
!setr 0 0
!skip 4
!peek
!setr 0 6
!setr 1 5
!skip 10000
!exit
EOF

    output=$( ../../build/Release/vm/cmd/vmctl ../../docs/spec/challenge < "${solution}" 2>&1 )
    codes=(`grep '^    [a-zA-Z]\{12\}$' <<< "$output"`)
    validate $1 ${codes[2]}
    rm ${solution}
}

# Brute-forcing in parallel
N=0x8000    # Maximum r7
G=32        # Number of simultaneous runs

# Running in batches
for g in $(seq 1 $G $N); do
    # Launch jobs
    for i in $( seq 0 $(($G - 1)) ); do
        attempt $(($g + $i)) &
        pids[${i}]=$!
    done

    # Wait for jobs to finish
    for pid in ${pids[*]}; do
        wait $pid
    done
done