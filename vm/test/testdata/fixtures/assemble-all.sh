cd $(dirname $0)
sources=$(find . -regex '.*\.as')

for src in $sources ; do
    dest=$(echo "$src" | sed 's#^\(.*\)\.as$#\1.in#')
    ../../../../build/Release/assembler/cmd/assemble "$src" "$dest"     \
        && echo "Built $dest"                                           \
        || echo "Could not build $dest" >&2
done
cd - > /dev/null