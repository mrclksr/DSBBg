# ABOUT

**DSBBg** is a simple Qt application to manage wallpapers, and to change your
desktop background using feh.

# INSTALLATION

## Dependencies

**DSBBg**
depends on devel/qt5-buildtools, devel/qt5-core, devel/qt5-linguisttools,
devel/qt5-qmake, x11-toolkits/qt5-gui, x11-toolkits/qt5-widgets, and
graphics/feh

## Building and installation

	# git clone https://github.com/mrclksr/DSBBg.git
	# git clone https://github.com/mrclksr/dsbcfg.git
	
	# cd DSBBg && qmake && make
	# make install

# SETUP

In order to set your desktop background when starting your window manager,
let `${HOME}/.xinitrc` start `${HOME}/.fehbg`.

# FILES
	${HOME}/.config/DSB/backgrounds
