wl_protocol_dir = wayland_protocols_dep.get_variable('pkgdatadir')
zen_protocol_dir = zen_protocols_dep.get_variable('pkgdatadir')
zwin_protocol_dir = zwin_protocols_dep.get_variable('pkgdatadir')

wayland_scanner = find_program(
	wayland_scanner_dep.get_variable('wayland_scanner'),
	native: true,
)

protocols = {
	'xdg-shell': wl_protocol_dir / 'stable/xdg-shell/xdg-shell.xml',

	'zen-desktop': zen_protocol_dir / 'unstable/zen-desktop.xml',

	'zwin': zwin_protocol_dir / 'zwin.xml',

  'wlr-layer-shell-unstable-v1': 'wlr-layer-shell-unstable-v1.xml',
}

protocols_code = {}
protocols_server_header = {}
protocols_client_header = {}
foreach name, path : protocols
	code = custom_target(
		name.underscorify() + '_c',
		input: path,
		output: '@BASENAME@-protocol.c',
		command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
	)

	server_header = custom_target(
		name.underscorify() + '_server_h',
		input: path,
		output: '@BASENAME@-protocol.h',
		command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
	)

	client_header = custom_target(
		name.underscorify() + '_client_h',
		input: path,
		output: '@BASENAME@-client-protocol.h',
		command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
		build_by_default: false,
	)

	protocols_code += { name: code }
	protocols_server_header += { name: server_header }
	protocols_client_header += { name: client_header }
endforeach

wlroots_srcs = [
	protocols_code['wlr-layer-shell-unstable-v1'],
	protocols_code['xdg-shell'],
	protocols_server_header['wlr-layer-shell-unstable-v1'],
	protocols_server_header['xdg-shell'],
]
