_wayland_protocols_dir = wayland_protocols_dep.get_variable('pkgdatadir')
_zwin_protocols_dir = zwin_protocols_dep.get_variable('pkgdatadir')
_scanner_path = wayland_scanner_dep.get_variable('wayland_scanner')
_wayland_scanner = find_program(_scanner_path)


_protocols = {
  'xdg-shell': join_paths(_wayland_protocols_dir, 'stable/xdg-shell/xdg-shell.xml'),
  'zwin': join_paths(_zwin_protocols_dir, 'zwin.xml'),
  'zwin-gles-v32': join_paths(_zwin_protocols_dir, 'zwin-gles-v32.xml'),
  'zwin-shell': join_paths(_zwin_protocols_dir, 'zwin-shell.xml'),
}

# protocols[<protocol-name>][<type>]
# <protocol-name> is one of the keys of _protocols, e.g. 'zen-desktop'.
# <type> is one of "private-code", "server-header", "client-header"
protocols = {}

# fill protocols
foreach key, xml : _protocols
  target = {}
  target += {'private-code': custom_target(
    xml.underscorify() + '_c',
    input: xml,
    output: '@BASENAME@-protocol.c',
    command: [_wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
  )}

  target += {'server-header': custom_target(
    xml.underscorify() + '_server_h',
    input: xml,
    output: '@BASENAME@-protocol.h',
    command: [_wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
  )}

  target += {'client-header': custom_target(
    xml.underscorify() + '_client_h',
    input: xml,
    output: '@BASENAME@-client-protocol.h',
    command: [_wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
  )}

  protocols += {key: target}
endforeach

zen_desktop_xml_abs_path = join_paths(meson.current_source_dir(), 'zen-desktop.xml')
