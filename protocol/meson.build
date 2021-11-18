dir_zgn_protocol_base = dep_zgn_protocol.get_variable('pkgdatadir')
prog_scanner = find_program(dep_wayland_scanner.get_variable('wayland_scanner'))

protocols = [ 'zigen', 'zigen-shell', 'zigen-opengl' ]

foreach proto : protocols
  xml_path = join_paths(dir_zgn_protocol_base, '@0@.xml'.format(proto))

  foreach  output_type : [ 'client-header', 'server-header', 'private-code' ]
    if output_type == 'client-header'
      output_file = '@0@-client-protocol.h'.format(proto)
    elif output_type == 'server-header'
      output_file = '@0@-server-protocol.h'.format(proto)
    else
      output_file = '@0@-protocol.c'.format(proto)
    endif

    var_name = output_file.underscorify()
    target = custom_target(
      '@0@ @1@'.format(proto, output_type),
      command: [ prog_scanner, output_type, '@INPUT@', '@OUTPUT@' ],
      input: xml_path,
      output: output_file,
    )

    set_variable(var_name, taregt)
  endforeach
endforeach
