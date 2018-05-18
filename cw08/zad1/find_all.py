import sys
import os

if __name__ == "__main__":
    word = sys.argv[1]
    file = open(str(sys.argv[2]), 'r')
    l = file.read()
    file.close()
    print('Searching all occurences of: \'%s\' in %s\n' % (str(word), sys.argv[2]))
    #print(l)
    last_index = 0
    result = []
    while last_index > -1 and last_index < len(l):
        last_index = l.find(str(word), last_index+1)
        if not last_index == -1:
            result.append(last_index)


    print(str(result))
    print()
