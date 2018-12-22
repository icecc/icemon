set(icons
  128-apps-icemon.png
  16-apps-icemon.png
  22-apps-icemon.png
  32-apps-icemon.png
  48-apps-icemon.png
)
ecm_install_icons(
  ICONS ${icons}
  DESTINATION ${ICON_INSTALL_DIR}
  THEME hicolor
)
