#!/bin/bash
flags=(
  -boot d                      # Boot from the CD-ROM
  -cdrom kernel.iso                    # Specify the CD-ROM image to use (ISO file)
  #-drive format=raw,file=kernel.iso
  -D log.txt
  -d int
  
  # -S
  # -s
 # -nographic
  #-drive if=pflash,unit=0,format=raw,file=/usr/share/ovmf/OVMF.fd,readonly=on
  -M smm=off
  -no-reboot
  -m 256M
  -no-shutdown
  -serial file:/dev/stdout
  -monitor stdio
)
qemu-system-x86_64 "${flags[@]}"


#target remote localhost:1234
