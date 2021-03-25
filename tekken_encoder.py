import numpy
import sys


def main():

    if len(sys.argv) != 4:
        print('There are three arguments: input_mcs input_binary output_mcs')
        return

    with open(sys.argv[2], 'rb') as fp:
        b = fp.read()


    if len(b) != 508:
        print(f'binary should have 508 bytes has {len(b)}')
        return


    data = [0, *b]
    data = list(map(lambda x: numpy.uint8(x), data))

    while len(data) < 512:
        data.append(numpy.uint8(0))

    acc = numpy.uint8(0)

    data_ptr_index = 1
    while data_ptr_index < 512:
        cur_byte = data[data_ptr_index]
        data_ptr_index += 1
        check_byte = acc + cur_byte
        acc += cur_byte

        
    data[0] = -check_byte
    current_check_byte = data[0]
    data_ptr_index = 1

    while data_ptr_index < 512:

        current_check_byte = data[data_ptr_index] + numpy.uint8(5) * current_check_byte + numpy.uint8(1)
        data[data_ptr_index] = current_check_byte
        data_ptr_index += 1

    with open(sys.argv[1], 'rb') as fp:
        mcs = fp.read()
        mcs = list(mcs)


    for i in range(512):
        mcs[0x280+i] = data[i]

    with open(sys.argv[3], 'wb') as fp:
        fp.write(bytes(mcs))

    print('Succesful encoding')


if __name__ == '__main__':
    main()
