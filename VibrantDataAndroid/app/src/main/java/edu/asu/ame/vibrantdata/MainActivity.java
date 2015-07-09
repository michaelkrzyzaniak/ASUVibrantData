package edu.asu.ame.vibrantdata;

//sudo tcpdump -l | nc -l 6543

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.media.AudioTrack;
import android.media.AudioFormat;
import android.media.AudioManager;

import java.io.IOException;
import java.net.*;
import java.util.*;
import android.widget.ToggleButton;
import com.illposed.osc.*;

/*----------------------------------------------------------------------------------------*/
public class MainActivity extends ActionBarActivity {
    static final int    AUDIO_FRAME_RATE             = 44100;
    static final double AUDIO_TWO_PI_OVER_FRAME_RATE = 2 * Math.PI / AUDIO_FRAME_RATE;

    static final double SYNTH_MIN_GAIN               = -24.0;
    static final double SYNTH_PULSE_DURATION         = 0.08;
    static final double SYNTH_RESONANT_FREQUENCY     = 88.0;

    //emulator receives on 10.0.2.15
    static final String OSC_SEND_IP                  = "10.0.2.2";
    static final int    OSC_SEND_PORT                = 6542;
    static final int    OSC_RECEIVE_PORT             = 6543;

    Thread       audio_thread                        ;
    boolean      audio_thread_is_running              = false;
    boolean      audio_thread_should_continue_running = false;
    AudioTrack   audio_track                         ;
    int          audio_buffer_size                   ;

    OSCPortOut   osc_sender                          ;
    OSCPortIn    osc_receiver                        ;
    OSCListener  osc_listener                        ;

    //audio synth variables
    double       amp                                 ;
    double       carrier_freq                        ;
    double       carrier_phase                       ;
    int          pulse_duration, pulse_spacing       ;
    int          pulse_counter                       ;
    double       vibration_magnitude                 ;

    Thread       test_thread;

    /*-___-___--_--_---__--__-----_---_------------_------------------------------------------
      / __|   \| |/ / |  \/  |___| |_| |_  ___  __| |___
      \__ \ |) | ' <  | |\/| / -_)  _| ' \/ _ \/ _` (_-<
      |___/___/|_|\_\ |_|  |_\___|\__|_||_\___/\__,_/__/
    ----------------------------------------------------------------------------------------*/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        audio_buffer_size = AudioTrack.getMinBufferSize(AUDIO_FRAME_RATE,
               AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_FLOAT);

        audio_track = new AudioTrack(AudioManager.STREAM_MUSIC, AUDIO_FRAME_RATE,
                AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_FLOAT, audio_buffer_size,
                AudioTrack.MODE_STREAM);

        audio_thread = new Thread() {
            public void run() {
                 audio_thread_run_loop();
            }
        };
        audio_thread.setPriority(Thread.MAX_PRIORITY);

        synth_set_carrier_freq  (SYNTH_RESONANT_FREQUENCY);
        synth_set_gain          (SYNTH_MIN_GAIN);
        synth_set_pulse_duration(SYNTH_PULSE_DURATION);
        synth_set_pulse_spacing (1);
        synth_set_vibration_magnitude(0.0);

        //need to check this error handling...
        //I don't fully understand how Java works...
        try {
            osc_receiver = new OSCPortIn(OSC_RECEIVE_PORT);
            osc_listener = new OSCListener(){
                @Override
                public void acceptMessage(Date time, OSCMessage message){osc_receive_magnitude(message);
                }
            };
            osc_receiver.addListener("/asu/vibrant/magnitude", osc_listener);
            osc_receiver.startListening();
        }catch (SocketException e) {
            System.out.println("UNABLE TO MAKE OSC IN PORT");
        }
        try{
            InetAddress addr = InetAddress.getByName(OSC_SEND_IP);
            osc_sender = new OSCPortOut(addr, OSC_SEND_PORT);
        }catch (UnknownHostException e){
            System.out.println("UNABLE TO GET INET ADDRESS");
            e.printStackTrace();
        } catch (SocketException e) {
            System.out.println("UNABLE TO MAKE OSC SEND PORT");
            e.printStackTrace();
        }

        synth_play();
    }

    /*----------------------------------------------------------------------------------------*/
    @Override
    public void onDestroy(){
        synth_stop();
        audio_track.release();
        super.onDestroy();
    }

    /*----------------------------------------------------------------------------------------*/
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    /*----------------------------------------------------------------------------------------*/
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    /*-_----------_-_-------___----------_---_------------------------------------------------
      /_\ _  _ __| (_)___  / __|_  _ _ _| |_| |_
     / _ \ || / _` | / _ \ \__ \ || | ' \  _| ' \
    /_/ \_\_,_\__,_|_\___/ |___/\_, |_||_\__|_||_|
                                |__/
    ----------------------------------------------------------------------------------------*/
    public void synth_play(){
        if(!audio_thread_is_running) {
            audio_thread_should_continue_running = true;
            audio_thread.start();
        }
    }

    /*----------------------------------------------------------------------------------------*/
    public void synth_stop(){
        if(audio_thread_is_running){
            audio_thread_should_continue_running = false;
            try {
                audio_thread.join();
            }
            catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    /*----------------------------------------------------------------------------------------*/
    public void synth_set_carrier_freq(double cps) {
        carrier_freq = cps * AUDIO_TWO_PI_OVER_FRAME_RATE;
    }

    /*----------------------------------------------------------------------------------------*/
    public double synth_get_carrier_freq() {
        return carrier_freq / AUDIO_TWO_PI_OVER_FRAME_RATE;
    }

    /*----------------------------------------------------------------------------------------*/
    public void synth_set_pulse_duration(double seconds) {
        pulse_duration = (int)(seconds * AUDIO_FRAME_RATE + 0.5);
    }

    /*----------------------------------------------------------------------------------------*/
    double synth_get_pulse_duration() {
        return pulse_duration / AUDIO_FRAME_RATE;
    }

    /*----------------------------------------------------------------------------------------*/
    public void synth_set_pulse_spacing(double seconds) {
        pulse_spacing = (int)(seconds * AUDIO_FRAME_RATE + 0.5);
    }

    /*----------------------------------------------------------------------------------------*/
    public double synth_get_pulse_spacing() {
        return pulse_spacing / AUDIO_FRAME_RATE;
    }

    /*----------------------------------------------------------------------------------------*/
    public void synth_set_gain(double decibels) {
        //AU_CONSTRAIN(decibels, SYNTH_MIN_GAIN, 0);
        decibels = (decibels < 0) ? 0 : (decibels > 1) ? 0 : decibels;
        if(decibels == SYNTH_MIN_GAIN)
            amp = 0;
        else
            amp = Math.pow(10.0, decibels / 10.0);
    }

    /*----------------------------------------------------------------------------------------*/
    public double synth_get_gain() {
        double result;
        if(amp == 0)
            result = SYNTH_MIN_GAIN;
        else
            result = 10 * Math.log10(amp);

        return result;
    }

    /*----------------------------------------------------------------------------------------*/
    public double synth_scalef(double x, double in_min, double in_max,
                               double out_min, double out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    /*----------------------------------------------------------------------------------------*/
    public void    synth_set_vibration_magnitude(double coefficient) {
        //AU_CONSTRAIN(coefficient, 0.0, 1.0);
        coefficient = (coefficient < 0) ? 0 : (coefficient > 1) ? 0 : coefficient;
        vibration_magnitude = coefficient;

        double decibels = synth_scalef(coefficient, 0, 1, SYNTH_MIN_GAIN, 0);
        double spacing     = synth_scalef(coefficient, 0, 1, 2, SYNTH_PULSE_DURATION);

        synth_set_gain(decibels);
        synth_set_pulse_spacing(spacing);
    }

    /*----------------------------------------------------------------------------------------*/
    public double  synth_get_vibration_magnitude() {
        return vibration_magnitude;
    }

    /*----------------------------------------------------------------------------------------*/
    public void audio_thread_run_loop() {
        audio_thread_is_running = true;
        audio_track.play();

        int i;
        float audio_buffer[] = new float[audio_buffer_size];
        float sample;

        while(audio_thread_should_continue_running){
            //mutex lock
            for(i=0; i < audio_buffer_size; i++){
                sample = (float)Math.sin(carrier_phase);
                sample *= (pulse_counter > pulse_duration) ? 0 : amp;

                pulse_counter++;
                pulse_counter %= pulse_spacing;
                carrier_phase += carrier_freq;
                audio_buffer[i] = sample;
            }
            audio_track.write(audio_buffer, 0, audio_buffer_size, AudioTrack.WRITE_BLOCKING);
        }
        audio_track.stop();
        audio_thread_is_running = false;
    }

    /*--___--___--___---___-----_------------__-----------------------------------------------
       / _ \/ __|/ __| |_ _|_ _| |_ ___ _ _ / _|__ _ __ ___
      | (_) \__ \ (__   | || ' \  _/ -_) '_|  _/ _` / _/ -_)
       \___/|___/\___| |___|_||_\__\___|_| |_| \__,_\__\___|
    ----------------------------------------------------------------------------------------*/
    public void osc_receive_magnitude(OSCMessage message){
        System.out.println("OSC RECEIVED");
        List args = message.getArguments();
        if(args.size() >= 1) {
            // if(typeof(args[0] == float))
            double magnitude = (double) args.get(0);
            synth_set_vibration_magnitude(magnitude);
        }
    }
    /*----------------------------------------------------------------------------------------*/
    public void osc_send_int_from_button(View button, String osc_address) {
        int val = ((ToggleButton) button).isChecked() ? 1 : 0;
        Object args[] = new Object[1];
        args[0] = new Integer(val);
        final OSCMessage message = new OSCMessage(osc_address, Arrays.asList(args));

        //send() throws exception if attempted on the main thread
        Thread osc_send_thread = new Thread() {
            public void run() {
                try {
                    osc_sender.send(message);
                } catch (Exception e) {
                    System.out.println("UNABLE TO SEND OSC MESSAGE");
                    e.printStackTrace();
                }
            }
        };
        osc_send_thread.start();
    }
    /*----------------------------------------------------------------------------------------*/
    public void set_wants_tcp(View view){
        osc_send_int_from_button(view, "/asu/vibrant/wants_tcp");
    }

    /*----------------------------------------------------------------------------------------*/
    public void set_wants_udp(View view){
        osc_send_int_from_button(view, "/asu/vibrant/wants_udp");
    }

    /*----------------------------------------------------------------------------------------*/
    public void set_wants_incoming(View view){
        osc_send_int_from_button(view, "/asu/vibrant/wants_incoming");
    }

    /*----------------------------------------------------------------------------------------*/
    public void set_wants_outgoing(View view){
        osc_send_int_from_button(view, "/asu/vibrant/wants_outgoing");
    }

    /*----------------------------------------------------------------------------------------*/
    public void set_wants_secure(View view){
        osc_send_int_from_button(view, "/asu/vibrant/wants_secure");
    }

    /*----------------------------------------------------------------------------------------*/
    public void set_wants_unsecure(View view){
        osc_send_int_from_button(view, "/asu/vibrant/wants_unsecure");
    }
}

