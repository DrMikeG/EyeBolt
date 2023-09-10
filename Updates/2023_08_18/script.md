Update

I had to go back and remind myself what I last shared about this project.

I showed the new turntable based on the 28BYK

I showed the capturing first light with the OV5640.

I showed the open CV calibration of the OV5640.

I can't believe I've not showed anything since then!?

I did a load of work, and intended to make a video of progress - but then didn't.
Instead I've been waiting for parts, which have all just arrived.

So, in this video I will bring us up to date with progress, and mention the new parts, and then I will dive into try to capture scans with the new parts, and figuring out what the next set of challenges are.

# EyeBolt #
 
Because of the field of view, I don't care about much of the image.
Pico has limit - Max 150kb of data
Ideally I'd like 1.5mb for 900x900 at full resolution - but equally I don't care about most of the those pixels, only the red ones.
 
Most experiments with 2560x1440 at high compression (24)
Binning is automatically enabled when output size is less 1440, so wondered where many of my pixels went when I used small output.
 
The library I am using is really useful for setting the 100s of registers and capturing the output data.
But output size is set using an enum indexing into a table.
 
I didn't really want to abandon the library, since its setting dozens of useful register values, but I want to use an output format that's not one of the pre-canned enum rows in the table.
 
The library itself is compiled into binary for efficiency, so I can't change it - but it turns out the table values are read-write so I can overwrite one row in the table.
 
I started off copying all the values from a working row and setting the offsets to shuffle a cropped window along.
 
The camera works in two modes depending on the output size - so the values are used differently depending, which took me a while to figure out.
 
With a bit of trial and error, managed to shift the capture window the be 950x950 (without scaling?)
 
Capturing fewer pixels allowed me to lower the compression from 24 to 6 (very close to 5)
 
The next issue to address is the noise.
The library is very video-focused and doesn't expose any of the setting related to the auto-exposure or auto-gain controls.
 
Here I have to go to the data sheet and start messing with individual bits in registers.

I like working with micro controllers because you can flash it fresh everytime.
When I get raspberry pi projects working, im never 100% sure I could repeat the process.
And I like working if python because the source is on the device. There is never a question if which compiler or libraries you used - because you can look and there they are.
Bit even with the ability to reset and trace what's going on, there is still the opportunity to be fooled by the things.


My approach here was to set the camera to a known good state, take a photo, then adjust some experimental settings, and take a second photos and compare.

Tried a setting to control something, and it made an amazing difference to the noise level.
Commited immediately.
Tried it again. Rubbish again.
Realised the sun had moved around and I had gotten up and shut the curtain.
Reducing the background light had changed the automatic exposure and or the automatic gain control.
So then I decided to turn off the automatic gain and exposure controls. this also seems to resolve the image corruption  or maybe not. This still catches me out.

So now I can capture a reasonble low noise image, will minimal compression, cropping it to the region of interest


Run_016.mp4 - hat spinning in box
Run_019.mp4 - hat spinning in box - white background

These are low res pictures, being taken slowly by the pico.
I'm stitching them together to make essentially a timelapse video.
In between each image, the motor advances the turn table.

Exposure testing with the red laser line.

Run 23 - testing different background illuminated exposure settings
Run 24 - testing different in box exposure settings

Best setting I could find were : run_024_exp_minus3.mp4

Pretty big deal:
Run_69 R2D2 in dark, lit by laser. 
Cropped image -much higher resolution & lower compression.

Distortion corrected.
Run 70 - different exposure

run_074_x_run_075.mp4 - hat scan, comparing exposure in the box to ambient.

In the day light image, I have much

 
Video of scan as point cloud - very noisy!

run_077_x_run_078.mp4 - R2 scan, comparing exposure in the box to ambient.

To reduce internal reflections and scatter, I've lined key parts of the box with the adhesive vinyl. 


run_079.mp4 - deck of cards scan - start of calibration.
Not calibrated in X/Y or Z!

I'm going to need more data! To average out reflectance noise, and also because the incident angle of the laser is not optimal for all surface positions.
The simplest way I could think to do this, is to add a second laser of another color. Although it turns out these are much harder to source than the red lasers.

I'd like to do a colour scan too - how would that work?

Well the pixels we use to know the shape are clearly obscured by the colour of the laser.
But once we know where the surface point is in 3D space, we should be able to look in another image, work out which new pixel represents that point on the surface, and sample a colour from that pixel from there.

Now the biggest obstacle I have to this is my turn table, which I noticed when making looping scan videos, does not rotate 360 degrees. In fact, it doesn't even reliably rotate.

I hadn't heard good things about the little 28BYK steppers, but it was so much simpler than having to build a gear box on a nema motor.
I spent a while sketching out designs for indexed gear and geneva drive - but it was hard to miniaturize as I wanted.

My approach for now is to swap to what I ope is a better motor.

This tiny DF Robot FIT0503 Micro Metal Geared Stepper Motor

I can use 3d printed adaptors to pop it in place of the 28BYK.

It has less backlash, and from these early experiments looks to step in much more reliable increments.

So, now I've