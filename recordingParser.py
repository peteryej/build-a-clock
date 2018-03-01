import csv

readFile = open('offset-delayRecording2.txt', 'r')
writeFile = open('offset-delayTable2.csv', 'w')
fieldnames = ['time (s)', 'offset (us)', 'delay (us)']
writer = csv.DictWriter(writeFile, fieldnames=fieldnames)
writer.writeheader()


counter = 0
time = None
offset = None
delay = None
for line in readFile:

	# if line.startswith('t0'):
	# 	counter += 1
	# 	time = line.split(' ')[1][:-1]
	if line.startswith('offset'):
		counter += 10
		offset = line.split(' ')[-2]
	if line.startswith('delay'):
		delay = line.split(' ')[-2]
	if offset and delay:

		writer.writerow({'time (s)':counter, 'offset (us)':offset, 'delay (us)':delay})
		time = None
		offset = None
		delay = None

readFile.close()
writeFile.close()
