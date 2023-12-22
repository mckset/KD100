FLAGS = -lusb-1.0
USER = $(shell id -u)
DIR = $(shell pwd)
HOME = "/home/"$(shell logname)

install:
	@if [ "${USER}" = "0" ]; then\
		${CC} KD100.c ${FLAGS} -o /bin/KD100;\
		mkdir "${HOME}/.config/KD100";\
		cp "default.cfg" "${HOME}/.config/KD100/";\
		chmod a+wr "${HOME}/.config/KD100/default.cfg";\
		echo "Default config file is located in: ${HOME}/.config/KD100/";\
	else\
		${CC} KD100.c ${FLAGS} -o KD100;\
	fi
