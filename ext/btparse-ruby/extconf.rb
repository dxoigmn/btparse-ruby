#!/usr/bin/ruby

require 'mkmf'

$CFLAGS << ' -std=c99'

dir_config('btparse')
have_library('btparse') or exit
have_header('btparse.h') or exit

create_makefile 'btparse_ruby'
