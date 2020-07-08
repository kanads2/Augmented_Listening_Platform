import numpy as np
import scipy.io as sio
import ez_beamform as ez

contents = sio.loadmat("/Users/kanadsarkar/ECE/audio/impulse_responses_2.mat")
impulse_responses = np.array(contents['impulse_responses'])
contents = sio.loadmat("/Users/kanadsarkar/ECE/audio/noise_sample.mat")
noise = np.array(contents['noise_sample'])

interference_irs = np.stack((impulse_responses[:,:,4],impulse_responses[:,:,7]), axis = -1)
target_ir = impulse_responses[:,:,1].reshape((4096,8,1))

print(noise.shape)
np.savetxt("target_ir.txt", target_ir[:,:,0])
np.savetxt("itf_ir_0.txt", interference_irs[:,:,0])
np.savetxt("itf_ir_1.txt", interference_irs[:,:,1])
np.savetxt("noise.txt", noise)

#np.savetxt()
#test_1 = ez.beamformer_binaural(target_ir, interference_irs)
test_2 = ez.beamformer_binaural(target_ir, 1)
   
#sio.savemat("/Users/kanadsarkar/ECE/audio/beam.mat", {'beamformer1_py': test_1 , 'beamformer2_py': test_2})

#NOTE: USE THE NON BINAUARAL VERISON, AND ALSO SAVE YOUR MAT FILES IN A VERSION BEFORE MATLAB's 7.3
