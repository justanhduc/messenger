import socket
import sys

from messenger_client import utils


def main():
    try:
        client = utils.MessengerClient(sys.argv)
        client.exec()
    except (socket.timeout, KeyboardInterrupt):
        pass


if __name__ == '__main__':
    main()
