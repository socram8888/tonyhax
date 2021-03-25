import numpy
import sys


def main():

    if len(sys.argv) != 3:
        print(sys.argv)
        print('There are two arguments: input output')
        return

    with open(sys.argv[1], 'rb') as fp:
        b = fp.read()

    b = b[0x280:]
    b = b[:512]



    b = list(map(lambda x: numpy.uint8(x), b))
    counter = 1
    check_byte = b[0]
    data_ptr_index = 1
    previous_byte = b[0]

    while counter < 512:
        counter += 1
        times_five = numpy.uint8(5) * previous_byte

        previous_byte = b[data_ptr_index]
        transformer = b[data_ptr_index] - (times_five + numpy.uint8(1))
        check_byte += transformer
        b[data_ptr_index] = transformer
        data_ptr_index += 1


    if check_byte:
        print('Error decoding the save file')
    else:
        print('Successful decoding')

        part1 = b[1:]
        part1 = part1[:44]

        part2 = b[45:]
        part2 = part2[:464]

        b = [*part1, *part2]
        with open(sys.argv[2], 'wb') as fp:
            fp.write(bytes(b))



if __name__ == '__main__':
    main()
