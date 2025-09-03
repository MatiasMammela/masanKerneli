xorriso -as mkisofs \
  -b limine-bios-cd.bin \
  -no-emul-boot \
  -boot-load-size 4 \
  -boot-info-table \
  --efi-boot limine-uefi-cd.bin \
  -efi-boot-part --efi-boot-image --protective-msdos-label \
  iso_root -o kernel.iso > /dev/null 2>&1
