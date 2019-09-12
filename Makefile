MULTITOUCH_LIBS=-F/System/Library/PrivateFrameworks -framework MultitouchSupport

FLASCHEN_TASCHEN_API_DIR=/Users/osnr/aux/flaschen-taschen/api
FLASCHEN_TASCHEN_API_LIBS=-I$(FLASCHEN_TASCHEN_API_DIR)/include -L$(FLASCHEN_TASCHEN_API_DIR)/lib -lftclient

trackpad-play: trackpad-play.cc
	clang++ -o trackpad-play $(MULTITOUCH_LIBS) $(FLASCHEN_TASCHEN_API_LIBS) $^

clean:
	rm trackpad-play
