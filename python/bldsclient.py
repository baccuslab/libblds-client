"""bldsclient.py
Python client to the Baccus Lab Data Server.
(C) 2017 Benjamin Naecker bnaecker@stanford.edu
"""

import socket
import struct

import numpy as np

_RECV_SIZE = 2048

class DataFrame():
    def __init__(self, start, stop, data):
        '''Create a DataFrame.

        The DataFrame object is used to collect individual chunks of data
        from the BLDS. It is a thin wrapper around a NumPy array, with a
        start and stop time, given as floats.

        Data itself is stored in a NumPy array, with shape given by 
        (nchannels, nsamples). The data may be accessed using the `data()`
        method.

        Parameters
        ----------

        start, stop : float
            The start and stop times of this chunk.

        data : numpy.ndarray
            The data for this chunk.
        '''
        self._start = start
        self._stop = stop
        self._data = data

    @staticmethod
    def deserialize(buf):
        """Deserialize a DataFrame from a byte array, e.g., from a message
        recieved from the BLDS.
        """
        start, stop, nsamples, nchannels = struct.unpack('<ffII', buf[:16])
        return DataFrame(start, stop, np.frombuffer(buf[16:], dtype=np.int16, 
            count=(nsamples*nchannels)).reshape(nchannels, nsamples))

    def serialize(self):
        """Serialize a DataFrame to bytes."""
        return struct.pack('<ffII', self._start, self._stop,
                *self._data.shape) + self._data.tobytes()

    def nchannels(self):
        """Return the number of channels in the chunk of data."""
        return self._data.shape[0]

    def nsamples(self):
        """Return the number of samples in the chunk of data."""
        return self._data.shape[1]

    def start(self):
        """Return the start time for this frame."""
        return self._start

    def stop(self):
        """Return the stop time for this frame."""
        return self._stop

    def data(self):
        """Return the data contained in the frame."""
        return self._data

class BldsError(Exception):
    """The BldsException class is used to indicate communication errors
    with the BLDS.
    """
    pass

class BldsClient():
    def __init__(self, hostname='localhost', port=12345):
        """Construct a client of the BLDS.

        Parameters
        ----------

        hostname : str, optional
            The hostname or IP address of the machine running the BLDS.
        
        port : int, optional
            The port number at which to connect. You shouldn't need to change this.
        """
        self._sock = None
        self._hostname = hostname
        self._port = port
        self._connected = False
        self._request_all_data = False

    def connect(self):
        """Connect to the BLDS."""
        if self._connected:
            return
        self._sock = socket.socket()
        self._sock.connect((self._hostname, self._port))
        self._connected = True

    def disconnect(self):
        """Disconnect from the BLDS."""
        if not self._connected:
            return
        self._sock.shutdown(socket.SHUT_RDWR)
        self._sock.close()
        self._sock = None
        self._connected = False

    def create_source(self, source_type='mcs', location=None):
        """Request that the BLDS create a data source of the given type.

        Parameters
        ----------
        
        source_type : str, optional
            The type of source to create. Can be one of {'mcs', 'hidens', 'file'}

        location : str, optional
            A location identifier for the source. If the type is 'mcs', this is
            ignored. If the type is 'file', this should be a filename that the BLDS
            can find. If the type is 'hidens', this should be the hostname or IP
            address for the machine running the HiDens server.
        """
        if source_type == 'file':
            if not isinstance(location, str):
                raise ValueError("'file' sources require a location")
        elif source_type == 'hidens':
            location = 'localhost' if location is None else location
        msg = '\n'.join(('create-source', source_type, location if location else ''))
        self._send_msg(msg.encode('utf8'))
        self._recv_msg()

    def delete_source(self):
        """Request that the BLDS delete the managed data source."""
        msg = b'delete-source\n'
        self._send_msg(msg)
        self._recv_msg()

    def start_recording(self):
        """Request that the BLDS start recording data to the current location for
        the current duration.
        """
        msg = b'start-recording\n'
        self._send_msg(msg)
        self._recv_msg()

    def stop_recording(self):
        """Request that the BLDS stop a running recording."""
        msg = b'stop-recording\n'
        self._send_msg(msg)
        self._recv_msg()

    def get_source(self, param):
        """Request the value of a named parameter of the data source."""
        msg = '\n'.join(('get-source', param + '\n' if not param.endswith('\n') else ''))
        self._send_msg(msg.encode('utf8'))
        return self._recv_msg()[-1]

    def get(self, param):
        """Request the value of a named parameter of the BLDS or its recording."""
        msg = '\n'.join(('get', param + '\n' if not param.endswith('\n') else ''))
        self._send_msg(msg.encode('utf8'))
        return self._recv_msg()[-1]

    def get_data(self, start=None, stop=None):
        """Request a chunk of data from the BLDS.

        If the client has requested all data (using the `get_all_data()` method),
        this will block until the next frame is available and return it. If the
        client has not requested all data, the `start` and `stop` times must be
        given. In this case, a single frame of data will be returned when it is
        received from the BLDS. This **will block** the client until the data 
        requested is actually available (i.e., until `stop` has come and gone).

        Parameters
        ----------

        start, stop : float, optional
            The start and stop times of the chunk to request. If the client has
            previously requested all data, these are ignored, and the next frame
            is returned when available.

        Returns
        -------

        frame : DataFrame
            The frame of data.
        """
        if self._request_all_data:
            return self._recv_msg() # Wait for data message available
        msg = b'get-data\n' + struct.pack('<ff', start, stop)
        self._send_msg(msg)
        return self._recv_msg()

    def request_all_data(self, request=True):
        """Request that the BLDS send all data as it becomes available. This
        allows clients to request data once, and receive it all, in chunks defined
        by the BLDS `'read-interval'` parameter. This is the fastest way to get
        data from the underlying source.

        Note that this method **must** be called **before** a recording is started.
        The server will then send data as soon as it is available, without requiring
        explicit requests from the client.
        """
        self._request_all_data = request
        msg = b'get-all-data\n' + struct.pack('<?', request)
        self._send_msg(msg)
        self._recv_msg()

    def set(self, param, value):
        """Set the value of a named parameter of the BLDS or its recording."""
        vals = [b'set', param.encode('utf8')]
        if param in ('save-file', 'save-directory'):
            vals.append(value.encode('utf8'))
        elif param in ('read-interval', 'recording-length'):
            vals.append(struct.pack('<I', value))
        else:
            raise BldsError('Unknown server parameter: {}'.format(param))
        self._send_msg(b'\n'.join(vals))
        self._recv_msg()
        
    def set_source(self, param, value):
        """Set the value of a named parameter of the managed data source."""
        vals = [b'set-source', param.encode('utf8')]
        if param == 'trigger':
            vals.append(value.encode('utf8'))
        elif param == 'adc-range':
            vals.append(struct.pack('<f', value))
        elif param in ('analog-output', 'configuration'):
            vals.append(struct.pack('<I', value.size) + value.tobytes())
        elif param == 'plug':
            vals.append(struct.pack('<I', value))
        else:
            raise BldsError('Unknown source parameter: {}'.format(param))
        self._send_msg(b'\n'.join(vals))
        self._recv_msg()

    def _decode_source_param(self, name, buf):
        if name in ('gain', 'adc-range', 'sample-rate'):
            return struct.unpack('<f', buf[:4])[0]
        elif name in ('trigger', 'connect-time', 'start-time', 
                'source-type', 'device-type', 'state', 'location'):
            return buf.decode('utf8')
        elif name == 'has-analog-output':
            return struct.unpack('<?', buf[:1])[0]
        elif name in ('nchannels', 'plug', 'chip-id', 'read-interval'):
            return struct.unpack('<I', buf[:4])[0]
        elif name == 'analog-output':
            size = struct.unpack('<I', buf[:4])[0]
            aout = np.frombuffer(buf[4:], dtype=np.double, count=size)
            return aout
        elif name == 'configuration':
            size = struct.unpack('<I', buf[:4])
            config = np.frombuffer(buf[4:], 
                    dtype=np.dtype({
                        'names' : 
                            ['index', 'xpos', 'x', 'ypos', 'y', 'label'],
                        'types' : 
                            [np.uint32, np.uint32, np.uint16, np.uint32, np.uint16, np.uint8]
                        }), count=size)
            return config

    def _decode_server_param(self, name, buf):
        if name in ('save-file', 'save-directory', 'source-type', 
                'source-location', 'start-time'):
            return buf.decode('utf8')
        elif name in ('recording-length', 'read-interval'):
            return struct.unpack('<I', buf[:4])[0]
        elif name == 'recording-position':
            return struct.unpack('<f', buf[:4])[0]
        elif name in ('recording-exists', 'source-exists'):
            return struct.unpack('<?', buf[:1])[0]

    def _send_msg(self, msg):
        if not self._connected:
            raise ConnectionError('Not connected to BLDS')
        self._sock.sendall(len(msg).to_bytes(4, 'little') + msg)

    def _recv_msg(self):
        buf = self._sock.recv(4, socket.MSG_WAITALL)
        if not buf:
            raise ConnectionError('BLDS closed the connection')

        msg_size = struct.unpack('<I', buf)[0]
        buf = self._sock.recv(msg_size, socket.MSG_WAITALL)

        msg_type, buf = buf.split(b'\n', maxsplit=1)
        if msg_type == b'error':
            raise BldsError(buf.decode('utf8'))
        elif msg_type == b'data':
            return DataFrame.deserialize(buf)
        success = struct.unpack('<?', buf[:1])[0]
        return self._parse_message_by_type(msg_type.decode('utf8'), success, buf[1:])

    def _parse_message_by_type(self, msg_type, success, buf):
        if not success:
            raise BldsError(buf.decode('utf8'))
        if msg_type == 'get':
            # message contains server param name, and value/error message
            name, buf = buf.split(b'\n', maxsplit=1)
            name = name.decode('utf8')
            return msg_type, name, self._decode_server_param(name, buf)
        elif msg_type == 'get-source':
            # message contains source param name, and value/error message
            name, buf = buf.split(b'\n', maxsplit=1)
            name = name.decode('utf8')
            return msg_type, name, self._decode_source_param(name, buf)
        else:
            # message contains possible error message or nothing if successful
            return msg_type, buf[1:].decode('utf8') if not success else ''

    def _verify_reply(self, expected):
        buf = self._sock.recv(4, socket.MSG_WAITALL)
        if len(buf) == 0:
            raise ConnectionError('BLDS closed the connection')
        size = struct.unpack('<I', buf)[0]
        buf = self._sock.recv(size, socket.MSG_WAITALL)
        if len(buf) == 0:
            raise ConnectionError('BLDS closed the connection')
        msg, buf = buf.split(b'\n', maxsplit=1)
        if msg != expected:
            raise ValueError('Message not received correctly, expected {}'.format(expected))
        success = struct.unpack('<?', buf[:1])[0]
        if not success:
            return success, buf[1:]
        return success, b''
