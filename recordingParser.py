import csv

readFile = open('offset-delayRecording.txt', 'r')
writeFile = open('offset-delayTable.csv', 'w')
fieldnames = ['time (s)', 'offset (us)', 'delay (us)']
writer = csv.DictWriter(writeFile, fieldnames=fieldnames)
writer.writeheader()

time = None
offset = None
delay = None
for line in readFile:
	if line.startswith('t0'):
		time = line.split(' ')[1][:-1]
	if line.startswith('offset'):
		offset = line.split(' ')[-2]
	if line.startswith('delay'):
		delay = line.split(' ')[-2]
	if time and offset and delay:
		writer.writerow({'time (s)':time, 'offset (us)':offset, 'delay (us)':delay})
		time = None
		offset = None
		delay = None

readFile.close()
writeFile.close()
