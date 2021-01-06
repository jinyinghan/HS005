cmd_/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o := mips-linux-gnu-gcc -Wp,-MD,/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/.rtw_p2p.o.d  -nostdinc -isystem /opt/mips-gcc472-glibc216-64bit/bin/../lib/gcc/mips-linux-gnu/4.7.2/include -I/home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include -Iarch/mips/include/generated  -Iinclude -I/home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi -Iarch/mips/include/generated/uapi -I/home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi -Iinclude/generated/uapi -include /home/lzf/T31/AP6PCT00/opensource/kernel/include/linux/kconfig.h -D__KERNEL__ -DVMLINUX_LOAD_ADDRESS=0xffffffff80010000 -DDATAOFFSET=0 -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -Wno-maybe-uninitialized -mno-check-zero-division -mabi=32 -mgp32 -mfp32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EL -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEL -D_MIPSEL -D__MIPSEL -D__MIPSEL__ -march=mips32 -Wa,-mips32 -Wa,--trap -I/home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/xburst/core/include -I/home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/xburst/soc-t31/include -I/home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic -Wno-array-bounds -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO -O1 -Wno-unused-variable -Wno-unused-value -Wno-unused-label -Wno-unused-parameter -Wno-unused-function -Wno-unused -I/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include -I/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/hal/phydm -I/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/platform -DCONFIG_RTL8188F -DCONFIG_MP_INCLUDED -DCONFIG_POWER_SAVING -DCONFIG_EFUSE_CONFIG_FILE -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_8189fs.map\" -DWIFIMAC_PATH=\"/data/wifimac.txt\" -DCONFIG_TRAFFIC_PROTECT -DCONFIG_LOAD_PHY_PARA_FROM_FILE -DREALTEK_CONFIG_PATH=\"\" -DCONFIG_TXPWR_BY_RATE_EN=1 -DCONFIG_TXPWR_LIMIT_EN=0 -DCONFIG_RTW_ADAPTIVITY_EN=0 -DCONFIG_RTW_ADAPTIVITY_MODE=0 -DCONFIG_RTW_SDIO_PM_KEEP_POWER -DCONFIG_REDUCE_TX_CPU_LOADING -DCONFIG_BR_EXT '-DCONFIG_BR_EXT_BRNAME="'br0'"' -DDM_ODM_SUPPORT_TYPE=0x04 -DCONFIG_LITTLE_ENDIAN -DCONFIG_MINIMAL_MEMORY_USAGE -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_PLATFORM_OPS  -DMODULE -mlong-calls  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(rtw_p2p)"  -D"KBUILD_MODNAME=KBUILD_STR(8189fs)" -c -o /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.c

source_/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o := /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.c

deps_/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o := \
    $(wildcard include/config/p2p.h) \
    $(wildcard include/config/wfd.h) \
    $(wildcard include/config/concurrent/mode.h) \
    $(wildcard include/config/tdls.h) \
    $(wildcard include/config/intel/widi.h) \
    $(wildcard include/config/method/pbc.h) \
    $(wildcard include/config/method/display.h) \
    $(wildcard include/config/ioctl/cfg80211.h) \
    $(wildcard include/config/debug/cfg80211.h) \
    $(wildcard include/config/drv/issue/prov/req.h) \
    $(wildcard include/config/cfg80211/onechannel/under/concurrent.h) \
    $(wildcard include/config/p2p/invite/iot.h) \
    $(wildcard include/config/p2p/ps.h) \
    $(wildcard include/config/p2p/op/chk/social/ch.h) \
    $(wildcard include/config/method/keypad.h) \
    $(wildcard include/config/dbg/p2p.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/drv_types.h \
    $(wildcard include/config/arp/keep/alive.h) \
    $(wildcard include/config/80211n/ht.h) \
    $(wildcard include/config/80211ac/vht.h) \
    $(wildcard include/config/beamforming.h) \
    $(wildcard include/config/prealloc/rx/skb/buffer.h) \
    $(wildcard include/config/wapi/support.h) \
    $(wildcard include/config/drvext.h) \
    $(wildcard include/config/mp/included.h) \
    $(wildcard include/config/br/ext.h) \
    $(wildcard include/config/iol.h) \
    $(wildcard include/config/bt/coexist.h) \
    $(wildcard include/config/1t1r.h) \
    $(wildcard include/config/2t2r.h) \
    $(wildcard include/config/tx/early/mode.h) \
    $(wildcard include/config/adaptor/info/caching/file.h) \
    $(wildcard include/config/layer2/roaming.h) \
    $(wildcard include/config/80211d.h) \
    $(wildcard include/config/special/setting/for/funai/tv.h) \
    $(wildcard include/config/ieee80211/band/5ghz.h) \
    $(wildcard include/config/load/phy/para/from/file.h) \
    $(wildcard include/config/multi/vir/ifaces.h) \
    $(wildcard include/config/auto/chnl/sel/nhm.h) \
    $(wildcard include/config/sdio/hci.h) \
    $(wildcard include/config/gspi/hci.h) \
    $(wildcard include/config/pci/hci.h) \
    $(wildcard include/config/dbg/counter.h) \
    $(wildcard include/config/dfs/master.h) \
    $(wildcard include/config/sdio/indirect/access.h) \
    $(wildcard include/config/usb/hci.h) \
    $(wildcard include/config/usb/vendor/req/mutex.h) \
    $(wildcard include/config/usb/vendor/req/buffer/prealloc.h) \
    $(wildcard include/config/intel/proxim.h) \
    $(wildcard include/config/mac/loopback/driver.h) \
    $(wildcard include/config/ieee80211w.h) \
    $(wildcard include/config/ap/mode.h) \
    $(wildcard include/config/bt/coexist/socket/trx.h) \
    $(wildcard include/config/gpio/api.h) \
    $(wildcard include/config/autosuspend.h) \
    $(wildcard include/config/pno/support.h) \
    $(wildcard include/config/pno/set/debug.h) \
    $(wildcard include/config/wowlan.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/drv_conf.h \
    $(wildcard include/config/android.h) \
    $(wildcard include/config/platform/android.h) \
    $(wildcard include/config/validate/ssid.h) \
    $(wildcard include/config/signal/display/dbm.h) \
    $(wildcard include/config/has/earlysuspend.h) \
    $(wildcard include/config/resume/in/workqueue.h) \
    $(wildcard include/config/android/power.h) \
    $(wildcard include/config/wakelock.h) \
    $(wildcard include/config/vendor/req/retry.h) \
    $(wildcard include/config/rtw/hiq/filter.h) \
    $(wildcard include/config/rtw/adaptivity/en.h) \
    $(wildcard include/config/rtw/adaptivity/mode.h) \
    $(wildcard include/config/rtw/adaptivity/dml.h) \
    $(wildcard include/config/rtw/adaptivity/dc/backoff.h) \
    $(wildcard include/config/rtw/adaptivity/th/l2h/ini.h) \
    $(wildcard include/config/rtw/adaptivity/th/edcca/hl/diff.h) \
    $(wildcard include/config/txpwr/by/rate/en.h) \
    $(wildcard include/config/txpwr/limit/en.h) \
    $(wildcard include/config/calibrate/tx/power/by/regulatory.h) \
    $(wildcard include/config/calibrate/tx/power/to/max.h) \
    $(wildcard include/config/rtw/target/tx/pwr/2g/a.h) \
    $(wildcard include/config/rtw/target/tx/pwr/2g/b.h) \
    $(wildcard include/config/rtw/target/tx/pwr/2g/c.h) \
    $(wildcard include/config/rtw/target/tx/pwr/2g/d.h) \
    $(wildcard include/config/rtw/target/tx/pwr/5g/a.h) \
    $(wildcard include/config/rtw/target/tx/pwr/5g/b.h) \
    $(wildcard include/config/rtw/target/tx/pwr/5g/c.h) \
    $(wildcard include/config/rtw/target/tx/pwr/5g/d.h) \
    $(wildcard include/config/rtw/amplifier/type/2g.h) \
    $(wildcard include/config/rtw/amplifier/type/5g.h) \
    $(wildcard include/config/rtw/rfe/type.h) \
    $(wildcard include/config/rtw/glna/type.h) \
    $(wildcard include/config/rtw/pll/ref/clk/sel.h) \
    $(wildcard include/config/rtl8812a.h) \
    $(wildcard include/config/rtl8821a.h) \
    $(wildcard include/config/rtl8814a.h) \
    $(wildcard include/config/deauth/before/connect.h) \
    $(wildcard include/config/wext/dont/join/byssid.h) \
    $(wildcard include/config/doscan/in/busytraffic.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/autoconf.h \
    $(wildcard include/config/rtl8188f.h) \
    $(wildcard include/config/recv/reordering/ctrl.h) \
    $(wildcard include/config/platform/intel/byt.h) \
    $(wildcard include/config/cfg80211/force/compatible/2/6/37/under.h) \
    $(wildcard include/config/set/scan/deny/timer.h) \
    $(wildcard include/config/nativeap/mlme.h) \
    $(wildcard include/config/hostapd/mlme.h) \
    $(wildcard include/config/find/best/channel.h) \
    $(wildcard include/config/tx/mcast2uni.h) \
    $(wildcard include/config/p2p/remove/group/info.h) \
    $(wildcard include/config/p2p/chk/invite/ch/list.h) \
    $(wildcard include/config/tdls/driver/setup.h) \
    $(wildcard include/config/tdls/autosetup.h) \
    $(wildcard include/config/tdls/autocheckalive.h) \
    $(wildcard include/config/tdls/ch/sw.h) \
    $(wildcard include/config/hwport/swap.h) \
    $(wildcard include/config/runtime/port/switch.h) \
    $(wildcard include/config/tsf/reset/offload.h) \
    $(wildcard include/config/scan/backop.h) \
    $(wildcard include/config/layer2/roaming/resume.h) \
    $(wildcard include/config/antenna/diversity.h) \
    $(wildcard include/config/sw/led.h) \
    $(wildcard include/config/xmit/ack.h) \
    $(wildcard include/config/active/keep/alive/check.h) \
    $(wildcard include/config/c2h/packet/en.h) \
    $(wildcard include/config/rf/gain/offset.h) \
    $(wildcard include/config/sdio/chk/hci/resume.h) \
    $(wildcard include/config/tx/aggregation.h) \
    $(wildcard include/config/sdio/rx/copy.h) \
    $(wildcard include/config/xmit/thread/mode.h) \
    $(wildcard include/config/sdio/tx/enable/aval/int.h) \
    $(wildcard include/config/skb/copy.h) \
    $(wildcard include/config/new/signal/stat/process.h) \
    $(wildcard include/config/embedded/fwimg.h) \
    $(wildcard include/config/file/fwimg.h) \
    $(wildcard include/config/long/delay/issue.h) \
    $(wildcard include/config/patch/join/wrong/channel.h) \
    $(wildcard include/config/attempt/to/fix/ap/beacon/error.h) \
    $(wildcard include/config/power/saving.h) \
    $(wildcard include/config/mp/iwpriv/support.h) \
    $(wildcard include/config/ips.h) \
    $(wildcard include/config/lps.h) \
    $(wildcard include/config/lps/lclk.h) \
    $(wildcard include/config/check/leave/lps.h) \
    $(wildcard include/config/lps/slow/transition.h) \
    $(wildcard include/config/detect/cpwm/by/polling.h) \
    $(wildcard include/config/lps/rpwm/timer.h) \
    $(wildcard include/config/lps/lclk/wd/timer.h) \
    $(wildcard include/config/ips/check/in/wd.h) \
    $(wildcard include/config/swlps/in/ips.h) \
    $(wildcard include/config/fwlps/in/ips.h) \
    $(wildcard include/config/gtk/ol.h) \
    $(wildcard include/config/gpio/wakeup.h) \
    $(wildcard include/config/debug.h) \
    $(wildcard include/config/debug/rtl871x.h) \
    $(wildcard include/config/proc/debug.h) \
    $(wildcard include/config/error/detect.h) \
    $(wildcard include/config/fw/c2h/debug.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_ic_cfg.h \
    $(wildcard include/config/multidrv.h) \
    $(wildcard include/config/rtl8188e.h) \
    $(wildcard include/config/rtl8192e.h) \
    $(wildcard include/config/rtl8723b.h) \
    $(wildcard include/config/rtl8703b.h) \
    $(wildcard include/config/rtl8822b.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/basic_types.h \
  include/generated/uapi/linux/version.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  include/uapi/linux/types.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/types.h \
    $(wildcard include/config/64bit/phys/addr.h) \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/types.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/posix_types.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/sgidefs.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/asm-generic/posix_types.h \
  include/linux/module.h \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/sig.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/debug/set/module/ronx.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
    $(wildcard include/config/illegal/pointer/value.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/const.h \
  include/linux/stat.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/stat.h \
  include/uapi/linux/stat.h \
  include/linux/time.h \
    $(wildcard include/config/arch/uses/gettimeoffset.h) \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/ring/buffer.h) \
  /opt/mips-gcc472-glibc216-64bit/bin/../lib/gcc/mips-linux-gnu/4.7.2/include/stdarg.h \
  include/linux/linkage.h \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modversions.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/linkage.h \
  include/linux/bitops.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/bitops.h \
    $(wildcard include/config/cpu/mipsr2.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/cavium/octeon.h) \
    $(wildcard include/config/sgi/ip28.h) \
    $(wildcard include/config/cpu/has/wb.h) \
    $(wildcard include/config/weak/ordering.h) \
    $(wildcard include/config/weak/reordering/beyond/llsc.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/addrspace.h \
    $(wildcard include/config/cpu/r8000.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic/spaces.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/kvm/guest.h) \
    $(wildcard include/config/dma/noncoherent.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/uapi/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  include/uapi/linux/swab.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/swab.h \
  include/linux/byteorder/generic.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/cpu-features.h \
    $(wildcard include/config/sys/supports/micromips.h) \
    $(wildcard include/config/cpu/mipsr2/irq/vi.h) \
    $(wildcard include/config/cpu/mipsr2/irq/ei.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/cpu.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/cpu-info.h \
    $(wildcard include/config/mips/mt/smp.h) \
    $(wildcard include/config/mips/mt/smtc.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/cache.h \
    $(wildcard include/config/mips/l1/cache/shift.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/xburst/soc-t31/include/cpu-feature-overrides.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/xburst/core/include/soc-ver.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/war.h \
    $(wildcard include/config/cpu/r4000/workarounds.h) \
    $(wildcard include/config/cpu/r4400/workarounds.h) \
    $(wildcard include/config/cpu/daddi/workarounds.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/xburst/soc-t31/include/war.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  include/asm-generic/bitops/sched.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/arch_hweight.h \
  include/asm-generic/bitops/arch_hweight.h \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/le.h \
  include/asm-generic/bitops/ext2-atomic.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/typecheck.h \
  include/linux/printk.h \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/broken/rodata.h) \
  include/linux/kern_levels.h \
  include/linux/dynamic_debug.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/uapi/linux/string.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/string.h \
    $(wildcard include/config/cpu/r3000.h) \
  include/linux/errno.h \
  include/uapi/linux/errno.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/errno.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/errno.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/asm-generic/errno-base.h \
  include/uapi/linux/kernel.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/sysinfo.h \
  include/linux/seqlock.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/context/tracking.h) \
    $(wildcard include/config/preempt/count.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/linux/bug.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/break.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/break.h \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/thread_info.h \
    $(wildcard include/config/page/size/4kb.h) \
    $(wildcard include/config/page/size/8kb.h) \
    $(wildcard include/config/page/size/16kb.h) \
    $(wildcard include/config/page/size/32kb.h) \
    $(wildcard include/config/page/size/64kb.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/processor.h \
    $(wildcard include/config/cavium/octeon/cvmseg/size.h) \
    $(wildcard include/config/mach/xburst.h) \
    $(wildcard include/config/pmon/debug.h) \
    $(wildcard include/config/mips/mt/fpaff.h) \
    $(wildcard include/config/cpu/has/prefetch.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/cachectl.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mipsregs.h \
    $(wildcard include/config/cpu/vr41xx.h) \
    $(wildcard include/config/mips/huge/tlb/support.h) \
    $(wildcard include/config/cpu/micromips.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/hazards.h \
    $(wildcard include/config/cpu/mipsr1.h) \
    $(wildcard include/config/mips/alchemy.h) \
    $(wildcard include/config/cpu/bmips.h) \
    $(wildcard include/config/cpu/loongson2.h) \
    $(wildcard include/config/cpu/r10000.h) \
    $(wildcard include/config/cpu/r5500.h) \
    $(wildcard include/config/cpu/xlr.h) \
    $(wildcard include/config/cpu/sb1.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/prefetch.h \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/irqflags.h \
    $(wildcard include/config/irq/cpu.h) \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/prove/rcu.h) \
  include/linux/rwlock_types.h \
  include/linux/spinlock_up.h \
  include/linux/rwlock.h \
  include/linux/spinlock_api_up.h \
  include/linux/atomic.h \
    $(wildcard include/config/arch/has/atomic/or.h) \
    $(wildcard include/config/generic/atomic64.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/atomic.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/cmpxchg.h \
  include/asm-generic/cmpxchg-local.h \
  include/asm-generic/atomic-long.h \
  include/asm-generic/atomic64.h \
  include/linux/math64.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/div64.h \
  include/asm-generic/div64.h \
  include/uapi/linux/time.h \
  include/linux/uidgid.h \
    $(wildcard include/config/uidgid/strict/type/checks.h) \
    $(wildcard include/config/user/ns.h) \
  include/linux/highuid.h \
  include/linux/kmod.h \
  include/linux/gfp.h \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/cma.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/isolation.h) \
    $(wildcard include/config/memcg.h) \
    $(wildcard include/config/compaction.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/have/memblock/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/no/bootmem.h) \
    $(wildcard include/config/numa/balancing.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/have/memoryless/nodes.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/have/arch/pfn/valid.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/wait.h \
  arch/mips/include/generated/asm/current.h \
  include/asm-generic/current.h \
  include/uapi/linux/wait.h \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/nodemask.h \
    $(wildcard include/config/movable/node.h) \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/linux/page-flags-layout.h \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  include/generated/bounds.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/page.h \
    $(wildcard include/config/cpu/mips32.h) \
  include/linux/pfn.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/io.h \
    $(wildcard include/config/pci.h) \
  include/asm-generic/iomap.h \
    $(wildcard include/config/has/ioport.h) \
    $(wildcard include/config/generic/iomap.h) \
  include/asm-generic/pci_iomap.h \
    $(wildcard include/config/no/generic/pci/ioport/map.h) \
    $(wildcard include/config/generic/pci/iomap.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/pgtable-bits.h \
    $(wildcard include/config/jzrisc/pep.h) \
    $(wildcard include/config/cpu/tx39xx.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic/ioremap.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic/mangle-port.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm-generic/memory_model.h \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/memory/hotremove.h) \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/have/bootmem/info/node.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/mutex/spin/on/owner.h) \
    $(wildcard include/config/have/arch/mutex/cpu/relax.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/rcu/torture/test.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/tree/preempt/rcu.h) \
    $(wildcard include/config/rcu/trace.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/rcu/user/qs.h) \
    $(wildcard include/config/tiny/rcu.h) \
    $(wildcard include/config/tiny/preempt/rcu.h) \
    $(wildcard include/config/debug/objects/rcu/head.h) \
    $(wildcard include/config/rcu/nocb/cpu.h) \
  include/linux/completion.h \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/rcutree.h \
  include/linux/workqueue.h \
    $(wildcard include/config/debug/objects/work.h) \
    $(wildcard include/config/freezer.h) \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/jiffies.h \
  include/linux/timex.h \
  include/uapi/linux/timex.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/param.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/param.h \
  include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  include/uapi/asm-generic/param.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/timex.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
    $(wildcard include/config/sched/book.h) \
    $(wildcard include/config/use/percpu/numa/node/id.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  include/linux/percpu.h \
    $(wildcard include/config/need/per/cpu/embed/first/chunk.h) \
    $(wildcard include/config/need/per/cpu/page/first/chunk.h) \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  arch/mips/include/generated/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/percpu-defs.h \
    $(wildcard include/config/debug/force/weak/per/cpu.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/topology.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/sysctl.h \
    $(wildcard include/config/sysctl.h) \
  include/linux/rbtree.h \
  include/uapi/linux/sysctl.h \
  include/linux/elf.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/elf.h \
    $(wildcard include/config/mips32/n32.h) \
    $(wildcard include/config/mips32/o32.h) \
    $(wildcard include/config/mips32/compat.h) \
  include/uapi/linux/elf.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/elf-em.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kobject_ns.h \
  include/linux/kref.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/tracepoint.h \
  include/linux/static_key.h \
  include/linux/jump_label.h \
    $(wildcard include/config/jump/label.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/module.h \
    $(wildcard include/config/cpu/mips32/r1.h) \
    $(wildcard include/config/cpu/mips32/r2.h) \
    $(wildcard include/config/cpu/mips64/r1.h) \
    $(wildcard include/config/cpu/mips64/r2.h) \
    $(wildcard include/config/cpu/r4300.h) \
    $(wildcard include/config/cpu/r4x00.h) \
    $(wildcard include/config/cpu/tx49xx.h) \
    $(wildcard include/config/cpu/r5000.h) \
    $(wildcard include/config/cpu/r5432.h) \
    $(wildcard include/config/cpu/r6000.h) \
    $(wildcard include/config/cpu/nevada.h) \
    $(wildcard include/config/cpu/rm7000.h) \
    $(wildcard include/config/cpu/loongson1.h) \
    $(wildcard include/config/cpu/xlp.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/uaccess.h \
    $(wildcard include/config/eva.h) \
  include/linux/utsname.h \
    $(wildcard include/config/uts/ns.h) \
    $(wildcard include/config/proc/sysctl.h) \
  include/linux/sched.h \
    $(wildcard include/config/sched/debug.h) \
    $(wildcard include/config/no/hz/common.h) \
    $(wildcard include/config/lockup/detector.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
    $(wildcard include/config/sched/autogroup.h) \
    $(wildcard include/config/virt/cpu/accounting/native.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/audit.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/fanotify.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/perf/events.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/cgroup/sched.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/rcu/boost.h) \
    $(wildcard include/config/compat/brk.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/virt/cpu/accounting/gen.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/have/hw/breakpoint.h) \
    $(wildcard include/config/uprobes.h) \
    $(wildcard include/config/bcache.h) \
    $(wildcard include/config/have/unstable/sched/clock.h) \
    $(wildcard include/config/irq/time/accounting.h) \
    $(wildcard include/config/no/hz/full.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/mm/owner.h) \
  include/uapi/linux/sched.h \
  include/linux/capability.h \
  include/uapi/linux/capability.h \
  include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/have/cmpxchg/double.h) \
    $(wildcard include/config/have/aligned/struct/page.h) \
    $(wildcard include/config/want/page/debug/flags.h) \
    $(wildcard include/config/kmemcheck.h) \
    $(wildcard include/config/aio.h) \
    $(wildcard include/config/mmu/notifier.h) \
    $(wildcard include/config/transparent/hugepage.h) \
  include/linux/auxvec.h \
  include/uapi/linux/auxvec.h \
  arch/mips/include/generated/uapi/asm/auxvec.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/asm-generic/auxvec.h \
  include/linux/page-debug-flags.h \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/page/guard.h) \
    $(wildcard include/config/page/debug/something/else.h) \
  include/linux/uprobes.h \
    $(wildcard include/config/arch/supports/uprobes.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mmu.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/ptrace.h \
    $(wildcard include/config/cpu/has/smartmips.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/isadep.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/ptrace.h \
  arch/mips/include/generated/asm/cputime.h \
  include/asm-generic/cputime.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/asm-generic/cputime_jiffies.h \
  include/linux/sem.h \
  include/uapi/linux/sem.h \
  include/linux/ipc.h \
  include/uapi/linux/ipc.h \
  arch/mips/include/generated/uapi/asm/ipcbuf.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/asm-generic/ipcbuf.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/sembuf.h \
  include/linux/signal.h \
    $(wildcard include/config/old/sigaction.h) \
  include/uapi/linux/signal.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/signal.h \
    $(wildcard include/config/trad/signals.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/signal.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/asm-generic/signal-defs.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/sigcontext.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/sigcontext.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/siginfo.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/uapi/asm-generic/siginfo.h \
  include/linux/pid.h \
  include/linux/proportions.h \
  include/linux/percpu_counter.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
    $(wildcard include/config/seccomp/filter.h) \
  include/uapi/linux/seccomp.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/seccomp.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/unistd.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/unistd.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/unistd.h \
  include/linux/rculist.h \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/resource.h \
  include/uapi/linux/resource.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/resource.h \
  include/asm-generic/resource.h \
  include/uapi/asm-generic/resource.h \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
    $(wildcard include/config/timerfd.h) \
  include/linux/timerqueue.h \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/latencytop.h \
  include/linux/cred.h \
    $(wildcard include/config/debug/credentials.h) \
    $(wildcard include/config/security.h) \
  include/linux/key.h \
  include/linux/selinux.h \
    $(wildcard include/config/security/selinux.h) \
  include/linux/llist.h \
    $(wildcard include/config/arch/have/nmi/safe/cmpxchg.h) \
  include/linux/nsproxy.h \
  include/linux/err.h \
  include/uapi/linux/utsname.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/osdep_service.h \
    $(wildcard include/config/use/vmalloc.h) \
    $(wildcard include/config/ap/wowlan.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/osdep_service_linux.h \
    $(wildcard include/config/net/radio.h) \
    $(wildcard include/config/wireless/ext.h) \
    $(wildcard include/config/tcp/csum/offload/tx.h) \
    $(wildcard include/config/efuse/config/file.h) \
    $(wildcard include/config/file.h) \
    $(wildcard include/config/usb/suspend.h) \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/failslab.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/slub_def.h \
    $(wildcard include/config/slub/stats.h) \
    $(wildcard include/config/memcg/kmem.h) \
    $(wildcard include/config/slub/debug.h) \
  include/linux/kmemleak.h \
    $(wildcard include/config/debug/kmemleak.h) \
  include/linux/netdevice.h \
    $(wildcard include/config/dcb.h) \
    $(wildcard include/config/wlan.h) \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/mac80211/mesh.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/net/ipgre.h) \
    $(wildcard include/config/ipv6/sit.h) \
    $(wildcard include/config/ipv6/tunnel.h) \
    $(wildcard include/config/rps.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/xps.h) \
    $(wildcard include/config/bql.h) \
    $(wildcard include/config/rfs/accel.h) \
    $(wildcard include/config/fcoe.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/libfcoe.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/net/dsa.h) \
    $(wildcard include/config/net/ns.h) \
    $(wildcard include/config/netprio/cgroup.h) \
    $(wildcard include/config/net/dsa/tag/dsa.h) \
    $(wildcard include/config/net/dsa/tag/trailer.h) \
    $(wildcard include/config/netpoll/trap.h) \
  include/linux/pm_qos.h \
    $(wildcard include/config/pm.h) \
    $(wildcard include/config/pm/runtime.h) \
  include/linux/miscdevice.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/major.h \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
    $(wildcard include/config/acpi.h) \
    $(wildcard include/config/pinctrl.h) \
    $(wildcard include/config/devtmpfs.h) \
    $(wildcard include/config/sysfs/deprecated.h) \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pinctrl/devinfo.h \
  include/linux/pm.h \
    $(wildcard include/config/vt/console/sleep.h) \
    $(wildcard include/config/pm/clk.h) \
    $(wildcard include/config/pm/generic/domains.h) \
  include/linux/ratelimit.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/device.h \
  include/linux/pm_wakeup.h \
  include/linux/delay.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/delay.h \
  include/linux/dmaengine.h \
    $(wildcard include/config/async/tx/enable/channel/switch.h) \
    $(wildcard include/config/rapidio/dma/engine.h) \
    $(wildcard include/config/dma/engine.h) \
    $(wildcard include/config/net/dma.h) \
    $(wildcard include/config/async/tx/dma.h) \
  include/linux/uio.h \
  include/uapi/linux/uio.h \
  include/linux/scatterlist.h \
    $(wildcard include/config/debug/sg.h) \
  include/linux/mm.h \
    $(wildcard include/config/x86.h) \
    $(wildcard include/config/ppc.h) \
    $(wildcard include/config/parisc.h) \
    $(wildcard include/config/metag.h) \
    $(wildcard include/config/ksm.h) \
    $(wildcard include/config/debug/vm/rb.h) \
    $(wildcard include/config/arch/uses/numa/prot/none.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/hibernation.h) \
    $(wildcard include/config/hugetlbfs.h) \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/range.h \
  include/linux/bit_spinlock.h \
  include/linux/shrinker.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/pgtable.h \
    $(wildcard include/config/cpu/supports/uncached/accelerated.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/pgtable-32.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/fixmap.h \
    $(wildcard include/config/eva/3gb.h) \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/pageflags/extended.h) \
    $(wildcard include/config/arch/uses/pg/uncached.h) \
    $(wildcard include/config/memory/failure.h) \
    $(wildcard include/config/swap.h) \
  include/linux/huge_mm.h \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/vm_event_item.h \
    $(wildcard include/config/migration.h) \
  arch/mips/include/generated/asm/scatterlist.h \
  include/asm-generic/scatterlist.h \
    $(wildcard include/config/need/sg/dma/length.h) \
  include/linux/dynamic_queue_limits.h \
  include/linux/ethtool.h \
  include/linux/compat.h \
    $(wildcard include/config/compat/old/sigaction.h) \
    $(wildcard include/config/odd/rt/sigaction.h) \
  include/uapi/linux/ethtool.h \
  include/linux/if_ether.h \
  include/linux/skbuff.h \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/nf/defrag/ipv4.h) \
    $(wildcard include/config/nf/defrag/ipv6.h) \
    $(wildcard include/config/xfrm.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/ipv6/ndisc/nodetype.h) \
    $(wildcard include/config/network/secmark.h) \
    $(wildcard include/config/network/phy/timestamping.h) \
    $(wildcard include/config/netfilter/xt/target/trace.h) \
  include/linux/kmemcheck.h \
  include/linux/net.h \
  include/linux/random.h \
    $(wildcard include/config/arch/random.h) \
  include/uapi/linux/random.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/ioctl.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/uapi/asm-generic/ioctl.h \
  include/linux/irqnr.h \
    $(wildcard include/config/generic/hardirqs.h) \
  include/uapi/linux/irqnr.h \
  include/linux/fcntl.h \
  include/uapi/linux/fcntl.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/fcntl.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/asm-generic/fcntl.h \
  include/uapi/linux/net.h \
  include/linux/socket.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/socket.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/socket.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/uapi/asm/sockios.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/sockios.h \
  include/uapi/linux/socket.h \
  include/linux/textsearch.h \
  include/net/checksum.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/checksum.h \
  include/linux/in6.h \
  include/uapi/linux/in6.h \
  include/linux/dma-mapping.h \
    $(wildcard include/config/has/dma.h) \
    $(wildcard include/config/arch/has/dma/set/coherent/mask.h) \
    $(wildcard include/config/have/dma/attrs.h) \
    $(wildcard include/config/need/dma/map/state.h) \
  include/linux/dma-attrs.h \
  include/linux/dma-direction.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/dma-mapping.h \
    $(wildcard include/config/sgi/ip27.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/dma-coherence.h \
  include/asm-generic/dma-coherent.h \
    $(wildcard include/config/have/generic/dma/coherent.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mach-generic/dma-coherence.h \
  include/asm-generic/dma-mapping-common.h \
  include/linux/dma-debug.h \
    $(wildcard include/config/dma/api/debug.h) \
  include/linux/netdev_features.h \
  include/net/flow_keys.h \
  include/uapi/linux/if_ether.h \
  include/net/net_namespace.h \
    $(wildcard include/config/ipv6.h) \
    $(wildcard include/config/ip/sctp.h) \
    $(wildcard include/config/ip/dccp.h) \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/wext/core.h) \
  include/net/netns/core.h \
  include/net/netns/mib.h \
    $(wildcard include/config/xfrm/statistics.h) \
  include/net/snmp.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/snmp.h \
  include/linux/u64_stats_sync.h \
  include/net/netns/unix.h \
  include/net/netns/packet.h \
  include/net/netns/ipv4.h \
    $(wildcard include/config/ip/multiple/tables.h) \
    $(wildcard include/config/ip/route/classid.h) \
    $(wildcard include/config/ip/mroute.h) \
    $(wildcard include/config/ip/mroute/multiple/tables.h) \
  include/net/inet_frag.h \
  include/net/netns/ipv6.h \
    $(wildcard include/config/ipv6/multiple/tables.h) \
    $(wildcard include/config/ipv6/mroute.h) \
    $(wildcard include/config/ipv6/mroute/multiple/tables.h) \
  include/net/dst_ops.h \
  include/net/netns/sctp.h \
  include/net/netns/dccp.h \
  include/net/netns/netfilter.h \
  include/linux/proc_fs.h \
  include/linux/fs.h \
    $(wildcard include/config/fs/posix/acl.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/fsnotify.h) \
    $(wildcard include/config/ima.h) \
    $(wildcard include/config/debug/writecount.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/fs/xip.h) \
  include/linux/kdev_t.h \
  include/uapi/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist_bl.h \
  include/linux/list_bl.h \
  include/linux/path.h \
  include/linux/radix-tree.h \
  include/linux/semaphore.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/fiemap.h \
  include/linux/migrate_mode.h \
  include/linux/percpu-rwsem.h \
  include/linux/blk_types.h \
    $(wildcard include/config/blk/cgroup.h) \
    $(wildcard include/config/blk/dev/integrity.h) \
  include/uapi/linux/fs.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/limits.h \
  include/linux/quota.h \
    $(wildcard include/config/quota/netlink/interface.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/projid.h \
  include/uapi/linux/quota.h \
  include/linux/nfs_fs_i.h \
  include/linux/netfilter.h \
    $(wildcard include/config/nf/nat/needed.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/if.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/hdlc/ioctl.h \
  include/linux/in.h \
  include/uapi/linux/in.h \
  include/uapi/linux/netfilter.h \
  include/net/netns/x_tables.h \
    $(wildcard include/config/bridge/nf/ebtables.h) \
  include/net/netns/xfrm.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/xfrm.h \
  include/linux/seq_file_net.h \
  include/linux/seq_file.h \
  include/net/dsa.h \
  include/net/netprio_cgroup.h \
  include/linux/cgroup.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/cgroupstats.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/taskstats.h \
  include/linux/prio_heap.h \
  include/linux/idr.h \
  include/linux/xattr.h \
  include/uapi/linux/xattr.h \
  include/linux/cgroup_subsys.h \
    $(wildcard include/config/cgroup/debug.h) \
    $(wildcard include/config/cgroup/cpuacct.h) \
    $(wildcard include/config/cgroup/device.h) \
    $(wildcard include/config/cgroup/freezer.h) \
    $(wildcard include/config/net/cls/cgroup.h) \
    $(wildcard include/config/cgroup/perf.h) \
    $(wildcard include/config/cgroup/hugetlb.h) \
    $(wildcard include/config/cgroup/bcache.h) \
  include/linux/hardirq.h \
  include/linux/ftrace_irq.h \
    $(wildcard include/config/ftrace/nmi/enter.h) \
  include/linux/vtime.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/hardirq.h \
  include/asm-generic/hardirq.h \
  include/linux/irq_cpustat.h \
  include/linux/irq.h \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/hardirqs/sw/resend.h) \
  include/linux/irqreturn.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/irq.h \
    $(wildcard include/config/i8259.h) \
    $(wildcard include/config/mips/mt/smtc/irqaff.h) \
    $(wildcard include/config/mips/mt/smtc/im/backstop.h) \
  include/linux/irqdomain.h \
    $(wildcard include/config/irq/domain.h) \
    $(wildcard include/config/of/irq.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/mipsmtregs.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/xburst/soc-t31/include/irq.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/irq_regs.h \
  include/linux/irqdesc.h \
    $(wildcard include/config/irq/preflow/fasteoi.h) \
    $(wildcard include/config/sparse/irq.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/hw_irq.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/neighbour.h \
  include/linux/netlink.h \
  include/net/scm.h \
    $(wildcard include/config/security/network.h) \
  include/linux/security.h \
    $(wildcard include/config/security/path.h) \
    $(wildcard include/config/security/network/xfrm.h) \
    $(wildcard include/config/securityfs.h) \
    $(wildcard include/config/security/yama.h) \
  include/uapi/linux/netlink.h \
  include/uapi/linux/netdevice.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/if_packet.h \
  include/linux/if_link.h \
  include/uapi/linux/if_link.h \
  include/linux/circ_buf.h \
  include/linux/etherdevice.h \
    $(wildcard include/config/have/efficient/unaligned/access.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/arch/mips/include/asm/unaligned.h \
  include/linux/unaligned/le_struct.h \
  include/linux/unaligned/packed_struct.h \
  include/linux/unaligned/be_byteshift.h \
  include/linux/unaligned/generic.h \
  include/linux/wireless.h \
  include/uapi/linux/wireless.h \
  include/net/iw_handler.h \
    $(wildcard include/config/wext/priv.h) \
  include/linux/if_arp.h \
    $(wildcard include/config/firewire/net.h) \
  include/uapi/linux/if_arp.h \
  include/linux/rtnetlink.h \
  include/uapi/linux/rtnetlink.h \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/if_addr.h \
  include/linux/interrupt.h \
    $(wildcard include/config/irq/forced/threading.h) \
    $(wildcard include/config/generic/irq/probe.h) \
  include/linux/ip.h \
  include/uapi/linux/ip.h \
  include/linux/kthread.h \
  include/linux/vmalloc.h \
  include/uapi/linux/limits.h \
  include/net/ieee80211_radiotap.h \
  include/linux/ieee80211.h \
    $(wildcard include/config/timeout.h) \
  include/net/cfg80211.h \
    $(wildcard include/config/nl80211/testmode.h) \
    $(wildcard include/config/cfg80211/wext.h) \
  include/linux/debugfs.h \
    $(wildcard include/config/debug/fs.h) \
  /home/lzf/T31/AP6PCT00/opensource/kernel/include/uapi/linux/nl80211.h \
  include/net/regulatory.h \
  include/linux/wakelock.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_byteorder.h \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/big/endian.h) \
    $(wildcard include/config/platform/mstar389.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/byteorder/little_endian.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/wlan_bssdef.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/wifi.h \
    $(wildcard include/config/rtl8712fw.h) \
    $(wildcard include/config/error.h) \
    $(wildcard include/config/method/flash.h) \
    $(wildcard include/config/method/ethernet.h) \
    $(wildcard include/config/method/label.h) \
    $(wildcard include/config/method/e/nfc.h) \
    $(wildcard include/config/method/i/nfc.h) \
    $(wildcard include/config/method/nfc.h) \
    $(wildcard include/config/method/vpbc.h) \
    $(wildcard include/config/method/ppbc.h) \
    $(wildcard include/config/method/vdisplay.h) \
    $(wildcard include/config/method/pdisplay.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/ieee80211.h \
    $(wildcard include/config/rtl8711fw.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/drv_types_linux.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_debug.h \
    $(wildcard include/config/p2p/wowlan.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_rf.h \
    $(wildcard include/config/.h) \
    $(wildcard include/config/1t.h) \
    $(wildcard include/config/2t.h) \
    $(wildcard include/config/1r.h) \
    $(wildcard include/config/2r.h) \
    $(wildcard include/config/1t2r.h) \
    $(wildcard include/config/turbo.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_ht.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_cmd.h \
    $(wildcard include/config/event/thread/mode.h) \
    $(wildcard include/config/c2h/wk.h) \
    $(wildcard include/config/h2clbk.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/cmd_osdep.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_security.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_xmit.h \
    $(wildcard include/config/usb/tx/aggregation.h) \
    $(wildcard include/config/platform/arm/sunxi.h) \
    $(wildcard include/config/platform/arm/sun6i.h) \
    $(wildcard include/config/platform/arm/sun7i.h) \
    $(wildcard include/config/platform/arm/sun8i.h) \
    $(wildcard include/config/platform/arm/sun50iw1p1.h) \
    $(wildcard include/config/platform/mstar.h) \
    $(wildcard include/config/single/xmit/buf.h) \
    $(wildcard include/config/sdio/tx/tasklet.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/xmit_osdep.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_recv.h \
    $(wildcard include/config/single/recv/buf.h) \
    $(wildcard include/config/tcp/csum/offload/rx.h) \
    $(wildcard include/config/recv/thread/mode.h) \
    $(wildcard include/config/usb/interrupt/in/pipe.h) \
    $(wildcard include/config/rx/indicate/queue.h) \
    $(wildcard include/config/bsd/rx/use/mbuf.h) \
    $(wildcard include/config/signal/scale/mapping.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/recv_osdep.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_efuse.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_sreset.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_intf.h \
    $(wildcard include/config/support/usb/int.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_com.h \
    $(wildcard include/config/background/noise/monitor.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/HalVerDef.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_pg.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_phy.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_phy_reg.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_com_reg.h \
    $(wildcard include/config/usedk.h) \
    $(wildcard include/config/no/usedk.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_com_phycfg.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/../hal/hal_com_c2h.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_com_h2c.h \
    $(wildcard include/config/ra/dbg/cmd.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/hal_com_led.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/../hal/hal_dm.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_qos.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_pwrctrl.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_mlme.h \
    $(wildcard include/config/dfs.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/mlme_osdep.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_io.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_ioctl.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_ioctl_set.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_ioctl_query.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_ioctl_rtl.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/osdep_intf.h \
    $(wildcard include/config/r871x/test.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/../os_dep/linux/rtw_proc.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/../os_dep/linux/ioctl_cfg80211.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/../os_dep/linux/rtw_cfgvendor.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_eeprom.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/sta_info.h \
    $(wildcard include/config/atmel/rc/patch.h) \
    $(wildcard include/config/auto/ap/mode.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_event.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_mlme_ext.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_ap.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_version.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_odm.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/../hal/phydm/phydm_types.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_p2p.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_mp.h \
    $(wildcard include/config/txt.h) \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_br_ext.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/ip.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/if_ether.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/ethernet.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/circ_buf.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/rtw_android.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/drv_types_sdio.h \
    $(wildcard include/config/platform/sprd.h) \
  include/linux/mmc/sdio_func.h \
  include/linux/mod_devicetable.h \
  include/linux/uuid.h \
  include/uapi/linux/uuid.h \
  include/linux/mmc/pm.h \
  include/linux/mmc/sdio_ids.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/sdio_osintf.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/sdio_ops.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/sdio_ops_linux.h \
  /home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/include/sdio_hal.h \

/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o: $(deps_/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o)

$(deps_/home/lzf/T31/AP6PCT00/driver/wifi/rtl8189FS_linux_v4.3.24.4_18988.20160812/core/rtw_p2p.o):
