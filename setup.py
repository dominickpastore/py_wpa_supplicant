from distutils.core import setup, Extension


setup(
    name="wpa_supplicant",
    version="0.1.0dev1",
    description="An module to interface with wpa_supplicant (without DBus)", 
    ext_modules=[Extension('wpa_supplicant',
        sources=['wpa_supplicant.c', 'wpa_supplicant/wpa_ctrl.c'],
        define_macros=[
            ('CONFIG_CTRL_IFACE', None),
            ('CONFIG_CTRL_IFACE_UNIX', None)
        ]
    )]
)
