cask "gcc-arm-embedded" do
  # Exists as a cask because it is impractical as a formula:
  # https://github.com/Homebrew/homebrew-core/pull/45780#issuecomment-569246452
  version "10.3-2021.07,10.14.6"
  sha256 "cae8b168ad16556ca204c0b441f9fb2c05be85362fe10e3b210be6b7177ef2d7"

  url "https://developer.arm.com/-/media/Files/downloads/gnu-rm/#{version.before_comma}/gcc-arm-none-eabi-#{version.before_comma}-mac-#{version.after_comma}-sha1.pkg"
  name "GCC ARM Embedded"
  desc "Pre-built GNU bare-metal toolchain for 32-bit Arm processors"
  homepage "https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm"

  livecheck do
    url "https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads"
    strategy :page_match do |page|
      match = page.match(/href=.*?gcc-arm-none-eabi-(\d+\.\d+-\d+\.\d+)-mac-(\d+\.\d+\.\d+)-sha1.pkg/i)
      "#{match[1]},#{match[2]}"
    end
  end

  pkg "gcc-arm-none-eabi-#{version.before_comma}-mac-#{version.after_comma}-sha1.pkg"
  binary "#{appdir}/ARM/bin/arm-none-eabi-addr2line"
  binary "#{appdir}/ARM/bin/arm-none-eabi-ar"
  binary "#{appdir}/ARM/bin/arm-none-eabi-as"
  binary "#{appdir}/ARM/bin/arm-none-eabi-c++"
  binary "#{appdir}/ARM/bin/arm-none-eabi-c++filt"
  binary "#{appdir}/ARM/bin/arm-none-eabi-cpp"
  binary "#{appdir}/ARM/bin/arm-none-eabi-elfedit"
  binary "#{appdir}/ARM/bin/arm-none-eabi-g++"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcc"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcc-10.3.1"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcc-ar"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcc-nm"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcc-ranlib"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcov"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcov-dump"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gcov-tool"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gdb"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gdb-add-index"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gdb-add-index-py"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gdb-py"
  binary "#{appdir}/ARM/bin/arm-none-eabi-gprof"
  binary "#{appdir}/ARM/bin/arm-none-eabi-ld"
  binary "#{appdir}/ARM/bin/arm-none-eabi-ld.bfd"
  binary "#{appdir}/ARM/bin/arm-none-eabi-lto-dump"
  binary "#{appdir}/ARM/bin/arm-none-eabi-nm"
  binary "#{appdir}/ARM/bin/arm-none-eabi-objcopy"
  binary "#{appdir}/ARM/bin/arm-none-eabi-objdump"
  binary "#{appdir}/ARM/bin/arm-none-eabi-ranlib"
  binary "#{appdir}/ARM/bin/arm-none-eabi-readelf"
  binary "#{appdir}/ARM/bin/arm-none-eabi-size"
  binary "#{appdir}/ARM/bin/arm-none-eabi-strings"
  binary "#{appdir}/ARM/bin/arm-none-eabi-strip"

  uninstall pkgutil: "gcc.arm-none-eabi-#{version[/^(\d{2})/]}",
            delete:  "/Applications/ARM"
end
