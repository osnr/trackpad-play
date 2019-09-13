MULTITOUCH_LIBS=-F/System/Library/PrivateFrameworks -framework MultitouchSupport

FLASCHEN_TASCHEN_API_DIR=/Users/osnr/aux/flaschen-taschen/api
FLASCHEN_TASCHEN_API_LIBS=-I$(FLASCHEN_TASCHEN_API_DIR)/include -L$(FLASCHEN_TASCHEN_API_DIR)/lib -lftclient

MINIFB_DIR=/Users/osnr/aux/minifb
MINIFB_LIBS=-framework Cocoa -framework QuartzCore -framework Metal -framework MetalKit -I$(MINIFB_DIR)/include -L$(MINIFB_DIR)/build -lminifb

trackpad-play: trackpad-play.cc
	clang++ -O3 -o trackpad-play $(MULTITOUCH_LIBS) $(FLASCHEN_TASCHEN_API_LIBS) $(MINIFB_LIBS) $^

clean:
	rm trackpad-play
