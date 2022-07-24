_wayland_protocols_dir = wayland_protocols_dep.get_variable('pkgdatadir')
_scanner_path = wayland_scanner_dep.get_variable('wayland_scanner')
_wayland_scanner = find_program(_scanner_path)


_protocols = [
  join_paths(_wayland_protocols_dir, 'stable/xdg-shell/xdg-shell.xml'),
]

_wayland_protocol_srcs = []
_wayland_protocol_headers = []

foreach xml : _protocols
  _wayland_protocol_srcs += custom_target(
    xml.underscorify() + '_server_c',
    input: xml,
    output: '@BASENAME@-protocol.c',
    command: [_wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
  )
  _wayland_protocol_headers += custom_target(
    xml.underscorify() + '_server_h',
    input: xml,
    output: '@BASENAME@-protocol.h',
    command: [_wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
  )
endforeach

_lib_server_protocols = static_library(
  'server_protocols',
  _wayland_protocol_srcs + _wayland_protocol_headers,
  dependencies: wayland_server_dep.partial_dependency(compile_args: true)
)

server_protocols = declare_dependency(
  link_with: _lib_server_protocols,
  sources: _wayland_protocol_headers,
)

zen_desktop_xml_abs_path = join_paths(meson.current_source_dir(), 'zen-desktop.xml')
