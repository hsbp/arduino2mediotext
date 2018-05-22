-module(parser).
-export([parse/1]).

parse(X) -> parse(X, []).

parse(<<>>, Acc) -> lists:reverse(Acc);
parse(<<3:2, RowStart:3, RowStop:3, BlockStart:4, BlockStop:4, PayloadRest/binary>>, Acc) ->
	PayloadLength = (RowStop - RowStart) * (BlockStop - BlockStart),
	<<Payload:PayloadLength/binary, Rest/binary>> = PayloadRest,
	parse(Rest, [{block_update, RowStart, RowStop, BlockStart, BlockStop, Payload} | Acc]);
parse(<<1:3, OpsOffset:5, Rest/binary>>, Acc) ->
	parse(Rest, [{ops_offset, OpsOffset} | Acc]);
parse(<<1:2, ScrollSpeed:6, Rest/binary>>, Acc) ->
	parse(Rest, [{scroll_speed, ScrollSpeed} | Acc]);
parse(<<2, DisplayOffset, Rest/binary>>, Acc) ->
	parse(Rest, [{display_offset, DisplayOffset} | Acc]).
