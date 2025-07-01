for file in shaders/*.vert; do
    glslc "$file" -o "shaders/$(basename "$file" .vert).vert.spv"
    echo "Compiled $(basename "$file") to $(basename "$file" .vert).vert.spv"
done

for file in shaders/*.frag; do
    glslc "$file" -o "shaders/$(basename "$file" .frag).frag.spv"
    echo "Compiled $(basename "$file") to $(basename "$file" .frag).frag.spv"
done