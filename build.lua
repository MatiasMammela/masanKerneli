
---Build the kernel
mbs = require("mbs")
local kernel = mbs.project("kernel")
local sources = mbs.glob_files("src/*.c","src/*.asm")
local headers = mbs.glob_dirs("headers")
mbs.sources(kernel,sources)
mbs.headers(kernel,headers)
mbs.cflags(kernel,"-ffreestanding", "-fno-pie", "-m64" ,"-mcmodel=kernel", "-mno-red-zone","-O0","-g","-fno-omit-frame-pointer","-fno-stack-protector")
mbs.lflags(kernel,"-ffreestanding", "-no-pie", "-m64","-T../linker.ld","-nostdlib","-nostartfiles","-fno-stack-protector")
mbs.build(kernel)
os.execute("ninja -C build")
os.execute("ninja -C build -t compdb > build/compile_commands.json")
os.execute("ln -sf build/compile_commands.json .")
mbs.copy("iso_root/boot/kernel/","build/kernel")

---Build the iso
os.execute("./xorriso.sh")
---Run Qemu
os.execute("./qemu.sh")
