clean:
	sudo rm -rf libelf tests_build

installcheckelf:
	python3 installcheckelf.py lcheck_pic || sudo apt-get install -y check
	python3 installcheckelf.py lelf || (git clone -q https://github.com/WolfgangSt/libelf.git libelf && cd libelf && ls -al && ./configure && make -s && sudo make install -s)

install: installcheckelf
	python3 prep_tests.py
	mkdir tests_build || true
	temp=$$(cd tests_build && cmake .. && make)
	python3 clean_tests.py

singletest: clean install
	./tests_build/check_tests $(FOO)

alltests: clean install
	sudo apt-get update -y
	sudo apt-get install valgrind check -y
	python3 runall.py
