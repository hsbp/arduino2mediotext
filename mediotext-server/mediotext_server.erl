-module(mediotext_server).
-export([start/0]).

-define(DEVICE, "/dev/ttyUSB0").
-define(SPEED, 19200).
-define(PORT, 42493).
-define(TIMEOUT, 1000 * 60 * 30). % 30 minutes

start() ->
	SerialPort = serial:start([{speed, ?SPEED}]),
	{ok, LSock} = gen_tcp:listen(?PORT, [binary, {reuseaddr, true}]),
	acceptor(LSock, SerialPort).

acceptor(LSock, SerialPort) ->
	{ok, Sock} = gen_tcp:accept(LSock),
	SerialPort ! {open, ?DEVICE},
	timer:sleep(2500),
	Processed = processor(Sock, SerialPort),
	SerialPort ! {close},
	acceptor(LSock, SerialPort).

processor(Sock, SerialPort) ->
	receive
		{tcp, Sock, Data} ->
			SerialPort ! {send, Data},
			processor(Sock, SerialPort);
		{tcp_closed, Sock} -> closed;
		{tcp_error, Sock, Reason} -> {error, Reason};
		{data, SerialData} ->
			gen_tcp:send(Sock, SerialData),
			processor(Sock, SerialPort)
	after ?TIMEOUT ->
		gen_tcp:close(Sock),
		timeout
	end.
