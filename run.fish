#!/usr/bin/env fish

clear

echo "Aurora Build and Run Script"
echo "======================================"

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

if not test -f "CMakeLists.txt"
    print_error "CMakeLists.txt not found. Please run this script from the Aurora project root directory."
    exit 1
end

if not test -d "build"
    print_status "Creating build directory..."
    mkdir build
end

cd build

print_status "Configuring CMake..."
if cmake ..
    print_success "CMake configuration completed"
    ln -sf build/compile_commands.json ../compile_commands.json
else
    print_error "CMake configuration failed"
    exit 1
end

print_status "Compiling Aurora..."
if make -j(nproc)
    print_success "Compilation completed successfully"
else
    print_error "Compilation failed"
    exit 1
end

if not test -f "debug_example/debug_example"
    print_error "Executable 'debug_example' not found after compilation"
    exit 1
end

print_success "Build process completed successfully!"

print_status "Running Aurora..."
echo "======================================"
echo ""

if ./debug_example/debug_example
    echo ""
    print_success "Aurora finished execution"
else
    echo ""
    print_warning "Aurora exited with error code $status"
end
