cd $(dirname $0)
sources=$(find . -regex '.*\.as')

BUILD_TYPE=${BUILD_TYPE:-Release}

for src in $sources ; do
    dest=$(echo "$src" | sed 's#^\(.*\)\.as$#\1.in#')
    "../../../../build/${BUILD_TYPE}/assembler/cmd/assemble" "$src" "$dest"     \
                && echo "Built $dest"                                           \
                || echo "Could not build $dest" >&2
done
cd - > /dev/null