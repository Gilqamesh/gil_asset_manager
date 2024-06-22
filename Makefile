cc            := clang++
common_src    := file_protocol.cpp
server_bin    := server
server_src    := server.cpp $(common_src)
server_obj    := $(patsubst %.cpp,%.o,$(server_src))
server_dps    := $(patsubst %.cpp,%.d,$(server_src))
server_lflags := -lssl -lcrypto
client_bin    := client
client_src    := client.cpp web.cpp $(common_src)
client_obj    := $(patsubst %.cpp,%.o,$(client_src))
client_dps    := $(patsubst %.cpp,%.d,$(client_src))
client_lflags := -lssl -lcrypto libraylib.a -lm
cflags        := -std=c++17 -Wall -Wextra -Werror -g
ast_mgr_src   := asset_manager.cpp
ast_mgr_obj   := $(patsubst %.cpp,%.o,$(ast_mgr_src))
ast_mgr_dps   := $(patsubst %.cpp,%.d,$(ast_mgr_src))
ast_mgr_lib   := libasset_manager.a

%.o: %.cpp
	$(cc) -o $@ -c $< $(cflags) -MMD -MP -MF $(<:.cpp=.d)

$(client_bin): $(client_obj) $(ast_mgr_lib)
	$(cc) -o $@ $^ $(client_lflags)

$(server_bin): $(server_obj)
	$(cc) -o $@ $^ $(server_lflags)

$(ast_mgr_lib): $(ast_mgr_obj)
	ar rsc $@ $^

.phony: all
all: client_rule
all: server_rule

.phony: clean
clean:
	- rm $(server_dps) $(client_dps) $(ast_mgr_dps) \
	$(server_obj) $(client_obj) $(ast_mgr_obj) \
	$(server_bin) $(client_bin) $(ast_mgr_lib)

.phony: re
re: clean
re: all

.phony: server_rule
server_rule: $(server_bin)

.phony: client_rule
client_rule: $(client_bin)

.phony: asset_manager
asset_manager: $(ast_mgr_lib)

-include $(server_dps)
-include $(client_dps)
-include $(ast_mgr_dps)
