Qastrocam is really a great tool, but sadly, it seams that Franck Sicard don't have the time maintein it. I needed
some new features (Serial SCmod support), and contacted F.Sicard, but he didn't respond, so i decided to write these
feature on my one, forking from de last F. Sicard version (4.1 pre 18072009), naming this version 4.2

Features : 

* SCmod Serial :

	- use the /dev/ttyS1 device
	- device can be changed with the -dx option
	- Levels are inverted

* file virtual telescope :

	- qastrocm tracking system can now generate iris compatible files
	- options : -t file -dt <filename>
	- you may use PEAS with these files

