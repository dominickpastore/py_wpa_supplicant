from distutils.core import setup, Extension


setup(
    name="wpa_supplicant",
    version="0.1.0dev1",
    description="An module to interface with wpa_supplicant (without DBus)", 
    ext_modules=[Extension('wpa_supplicant',
        sources=[
            'wpa_supplicant.c',
            'hostap/src/common/wpa_ctrl.c',
            'hostap/src/utils/os_unix.c',
            'hostap/src/utils/common.c',
            'hostap/src/utils/wpabuf.c',
            'hostap/src/utils/wpa_debug.c'
        ],
        #extra_compile_args=['-Og', '-g'],
        define_macros=[
            ('CONFIG_CTRL_IFACE', None),
            ('CONFIG_CTRL_IFACE_UNIX', None)
        ],
        include_dirs=['hostap/src', 'hostap/src/common', 'hostap/src/utils']
    )]
)
