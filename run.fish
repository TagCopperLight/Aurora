#!/usr/bin/env fish

# Aurora Build and Run Script (Fish Shell Version)
# This script handles the complete build process: CMake configuration, compilation, and execution

clear

echo "Aurora Build and Run Script"
echo "======================================"

# Function to print colored output
function print_status
    echo (set_color blue)"[INFO]"(set_color normal) $argv
end

function print_success
    echo (set_color green)"[SUCCESS]"(set_color normal) $argv
end

function print_warning
    echo (set_color yellow)"[WARNING]"(set_color normal) $argv
end

function print_error
    echo (set_color red)"[ERROR]"(set_color normal) $argv
end

# Check if we're in the correct directory
if not test -f "CMakeLists.txt"
    print_error "CMakeLists.txt not found. Please run this script from the Aurora project root directory."
    exit 1
end

# Step 0: Compile shaders
print_status "Compiling shaders..."
if test -d "aurora_app/shaders"
    set shader_compiled false
    
    # Compile vertex shaders
    for file in aurora_app/shaders/*.vert
        if test -f "$file"
            set output_file (string replace ".vert" ".vert.spv" "$file")
            print_status "Compiling vertex shader: "(basename "$file")
            if glslc "$file" -o "$output_file"
                set shader_compiled true
            else
                print_error "Failed to compile vertex shader: $file"
                exit 1
            end
        end
    end
    
    # Compile fragment shaders
    for file in aurora_app/shaders/*.frag
        if test -f "$file"
            set output_file (string replace ".frag" ".frag.spv" "$file")
            print_status "Compiling fragment shader: "(basename "$file")
            if glslc "$file" -o "$output_file"
                set shader_compiled true
            else
                print_error "Failed to compile fragment shader: $file"
                exit 1
            end
        end
    end
    
    if test "$shader_compiled" = "true"
        print_success "Shader compilation completed"
    else
        print_warning "No shaders found to compile"
    end
else
    print_warning "Shader directory not found: aurora_app/shaders"
end

# Create build directory if it doesn't exist
if not test -d "build"
    print_status "Creating build directory..."
    mkdir build
end

cd build

# Step 1: Configure CMake
print_status "Configuring CMake..."
if cmake ..
    print_success "CMake configuration completed"
else
    print_error "CMake configuration failed"
    exit 1
end

# Step 2: Compile the project
print_status "Compiling Aurora..."
if make -j(nproc)
    print_success "Compilation completed successfully"
else
    print_error "Compilation failed"
    exit 1
end

# Step 3: Check if executable exists
if not test -f "debug_example/debug_example"
    print_error "Executable 'debug_example' not found after compilation"
    exit 1
end

print_success "Build process completed successfully!"

cd ..

# Step 4: Run the program
print_status "Running Aurora..."
echo "======================================"
echo ""

# Run the executable
if ./build/debug_example/debug_example
    echo ""
    print_success "Aurora finished execution"
else
    echo ""
    print_warning "Aurora exited with error code $status"
end
