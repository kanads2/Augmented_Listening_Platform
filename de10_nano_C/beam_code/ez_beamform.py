import numpy as np
import scipy.io as sio


def stft(input_sound, dft_size, hop_size, zero_pad, window):        
    input_zero = np.zeros(len(input_sound) + dft_size - (len(input_sound) % dft_size), dtype = 'complex')
    input_zero[0:len(input_sound)] = input_sound
    frame_counter = dft_size/2
    f = []    
    f.append(np.zeros(dft_size))
    while(True):
        temp = window * input_zero[(int)(frame_counter) : (int) (frame_counter+dft_size)]
        f.append(  np.fft.fft( temp, zero_pad+dft_size))
        frame_counter += hop_size
        if(frame_counter > len(input_zero)-1.5*dft_size):
            break
    f_array = f[0]       
    for n in f[1:]:
        f_array = np.vstack((f_array, n))
    return f_array.T

def multiple_stft(input_sounds, dft_size = 1024): # the input_sounds is size sound_len X num_sounds
    hop_size = (int)(dft_size/2)
    num_channels = len(input_sounds[0])
    padding = np.zeros((dft_size, num_channels))
    x = np.concatenate((padding, input_sounds), axis = 0)
    x =  np.concatenate((x,padding), axis = 0)
    window = np.hanning(dft_size)
    scaling = 1/np.sqrt(np.sum(window **2))
    D = []
    x = x.T
    x = x * scaling
    for i in range(num_channels):
        D.append(stft(x[i], dft_size, hop_size, 0, window))
    D_array = np.array(D)    
    D_array = np.swapaxes(D_array, 1,2)
    return D_array[:,:,:((int)(dft_size/2) +1)] #dimensions num_channels X num_frames x num_freq

def beamformer_binaural(target_ir, noise ):
    MAX_LEN = 8192;
    SDW = 1e+1; # Emphasizing getting a correct output over noise suppression
    DIAGONAL_LOADING = 1e-2; # used to help with estimation errors
    ir_len, num_channels, num_outputs = target_ir.shape
    dft_len = min(ir_len*2, MAX_LEN)


    #normalize the impulse responses before we generate the fourier transform on the input sounds 
    for n in range( num_outputs):
        target_ir[:,:,n] = target_ir[:,:,n]/np.sqrt(np.mean(np.sum(target_ir[:,:,n]**2, axis = 0))) 
    Ht = np.fft.fft(target_ir, n = dft_len, axis = 0)
    Ht = np.swapaxes(Ht, 0, 1)
    Ht = np.swapaxes(Ht, 1, 2)

    #Covariance Matrix for noise or interference
    C = np.zeros((num_channels,num_channels,dft_len), dtype = 'complex');

    #Fourier transform for noise and intereferences
    Hi = None
    if(len(noise.shape) == 1):
        for f in range(dft_len):
            C[:,:,f] = np.identity(num_channels);

    else:
        if(len(noise.shape) >2 or noise.shape[0] <= ir_len): # It should enter here for interference suppression
            Hi = np.fft.fft(noise,n = dft_len, axis = 0)
            Hi = np.swapaxes(Hi, 0,1)
            Hi = np.swapaxes(Hi, 1,2)
            Hi = Hi/(np.sqrt(np.mean(np.sum(np.mean(np.abs(Hi) **2, axis = 0), axis = 0))))
        else: #it should enter here for noise suppression
            Hi = multiple_stft(noise, dft_len)
            Hi = np.concatenate((Hi,np.conj(np.flip(Hi[:,:,1:ir_len], axis =2))), axis = 2)
            Hi = Hi[:,:]/np.sqrt(np.sum(np.mean(np.abs(Hi)**2, axis = 0), axis = 0))

        for f in range(dft_len):
            a = np.matrix(Hi[:,:,f])
            C[:,:,f] = np.matmul(a, a.H)
            C[:,:,f] = C[:,:,f] + DIAGONAL_LOADING*np.identity(num_channels)

    '''
    # This is for NON-BINAURAL
    # USE THIS BLOCK FOR THE TESTING
    W = np.zeros((num_outputs,num_channels,dft_len), dtype = 'complex')
    for f in range(dft_len):
        for n in range(num_outputs):
            b = np.matrix(Ht[:,n,f])
            W[n,:,f] = Ht[0,n,f]*np.matmul(np.conj(b), np.linalg.inv(np.matmul(b.H, b).T+C[:,:,f]/SDW))
    W = np.swapaxes(W,0,2)
    '''

    # This is for BINAURAL Filter Calculation
    # DONT USE THIS BLOCK FOR TESTING, IF YOU MUST THEN USE W[:,0, :,:] ONLY
    W = np.zeros((num_outputs,2,num_channels,dft_len), dtype = 'complex')
    for f in range(dft_len):
        for n in range(num_outputs):
            b = np.matrix(Ht[:,n,f])
            #print(np.linalg.eig( np.linalg.inv(np.matmul(b.H, b).T+C[:,:,f]/SDW)))
            W[n,0,:,f] = Ht[0,n,f]*np.matmul(np.conj(b), np.linalg.inv(np.matmul(b.H, b).T+C[:,:,f]/SDW))
            W[n,1,:,f] = Ht[1,n,f]*np.matmul(np.conj(b), np.linalg.inv(np.matmul(b.H, b).T+C[:,:,f]/SDW))
    W = np.swapaxes(W,0,3)
    W = np.swapaxes(W,1,2)

    wi = np.fft.ifft(W, n=dft_len, axis = 0)
    wi = np.roll(wi, (int)(dft_len/4), axis = 0)
    wi = wi[0:(int)(dft_len/2), :,:] 
    #print(wi.shape[0:2])
    #wi = np.reshape(wi,wi.shape[0:3])
    #print(wi[1025][5][1][0].real)
    return wi.real
   #return 0
