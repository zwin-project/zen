wl_protocol_dir = wayland_protocols_dep.get_variable('pkgdatadir')

wayland_scanner = find_program(
	wayland_scanner_dep.get_variable('wayland_scanner'),
	native: true,
)

protocols = {
	'xdg-shell': wl_protocol_dir / 'stable/xdg-shell/xdg-shell.xml',

  'wlr-layer-shell-unstable-v1': 'wlr-layer-shell-unstable-v1.xml',
}

wlroots_srcs = []

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

	wlroots_srcs += code

	server_header = custom_target(
		name.underscorify() + '_server_h',
		input: path,
		output: '@BASENAME@-protocol.h',
		command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
	)

	wlroots_srcs += server_header

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
