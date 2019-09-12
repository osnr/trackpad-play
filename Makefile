trackpad-play: trackpad-play.c
	cc -o trackpad-play -F/System/Library/PrivateFrameworks -framework MultitouchSupport $^
