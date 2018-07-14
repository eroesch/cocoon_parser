// Processor function implementation

#include "processor.h"

#define ZBEE_TYPE 1
#define WIFI_TYPE 2
#define IPSH_TYPE 3
#define AUDI_TYPE 4

void pro_audio(SLOT *slot){
        unsigned int i = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        FRAME *_frame = slot->frame_array;
        double *val_array = (double*)calloc(slot->n, sizeof(double));
        while(i < slot->n){
                Audio_Frame *frm = (Audio_Frame*)_frame->frame_ptr;
                val_array[i] = frm->db;
                avg += frm->db;
                _frame = _frame->next;
                i++;
        }
        if (i != 0){
                avg = _math_average_dbl(val_array, i);
                stdev = _math_stdev(_math_variance_dbl(val_array, avg, i));
                avg_dev = _math_avg_dev_dbl(val_array, i);
                if (isnan(avg_dev)) avg_dev=0.0;
                if (isnan(stdev)) stdev = 0.0;
                double min = 0.0;
                double max = 0.0;
                int freq = _math_count_threshold(val_array, i, 0.9);
                _math_minmax_dbl(val_array, i, &min, &max);
                printf("%" PRIu64 ",%f,%f,%f,%f,%d\n", slot->slot_stop_time, avg, min, max, stdev, freq);
                avg = 0.0;
                stdev = 0.0;
                avg_dev = 0.0;
                free(val_array);
        }
}

void pro_zbee(SLOT *slot, GLOBAL_KNOWLEDGE *glob){
        unsigned int i = 0;
        unsigned int k = 0;
        unsigned int j = 0;
        unsigned int freq = 0;
        unsigned int f = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        unsigned int proceed = 0;
//      unsigned char flags = 0;
        FRAME *_frame = slot->frame_array;
        while (f < glob->Global_Frames->n){
        while (k < glob->Global_Sources->n){   // for every source
                while(j < glob->Global_Destinations->n){ // for every destination
                        _frame = slot->frame_array;
                        unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
			uint64_t *dif_array = (uint64_t*)calloc(slot->n, sizeof(uint64_t)); // latenc
                        while(i < slot->n){ // for each packet
                                ZigBee_Frame *frm = (ZigBee_Frame*)_frame->frame_ptr;
                                if (frm->frame_type == (*(glob->Global_Frames->array+f))) proceed = 1;
                                if (proceed && (*(glob->Global_Sources->array+k) == frm->src_id) && (*(glob->Global_Destinations->array+j) == frm->dst_id)){
					dif_array[freq] = frm->timestamp;	// ~ latec
                                        val_array[freq] = frm->packet_size;
                                        freq++;
                                        avg += frm->packet_size;
//                                        if (frm->flags == 1) printf("%" PRIu64 ",%04x,%04x,%d,%d,%d\n", slot->slot_stop_time, (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)),(*(glob->Global_Frames->array+j)),(*(glob->Global_Frames->array+f)), frm->packet_size, frm->flags);
//                                        if (frm->flags == 0) printf("%" PRIu64 ",%04x,%04x,%d,%d,%d\n", slot->slot_stop_time, (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)),(*(glob->Global_Frames->array+j)),(*(glob->Global_Frames->array+f)), frm->packet_size, frm->flags);
                                }
                                // assuming there is only 1 packet in the slot
                        //      flags = frm->flags;
                                _frame = _frame->next;
                                proceed = 0;
                                i++;
                        }
			unsigned int *latency_array = _math_generate_latency_array(dif_array, freq);//(unsigned int*)calloc(freq, sizeof(int)); // lat
			double avg_latency = _math_average(latency_array, freq-1); // lat
			double std_latency = _math_stdev(_math_variance(latency_array, avg, freq-1)); // lat
			unsigned int min_latency, max_latency; // late
			_math_minmax(latency_array, freq-1, &min_latency, &max_latency); // lat
                        avg = _math_average(val_array, freq);
                        stdev = _math_stdev(_math_variance(val_array, avg, freq));
                        avg_dev = _math_avg_dev(val_array, freq);
                        if (isnan(avg_dev)) avg_dev=0.0;
                        if (isnan(stdev)) stdev = 0.0;
                        unsigned int min = 0;
                        unsigned int max = 0;
                        _math_minmax(val_array, freq, &min, &max);
                        freq = 0;
                        avg = 0.0;
                        i = 0;
			free(dif_array);
			free(latency_array);
                        free(val_array);
                        j++;
                }
                j = 0;
                k++;
        }
        k = 0; j = 0;
        f++;
        }
        glob->Global_Sources->n = 0;
        glob->Global_Destinations->n = 0;
        glob->Global_Frames->n = 0;
        i = 0;
}

void pro_wifi(SLOT *slot, GLOBAL_KNOWLEDGE *glob){
        unsigned int i = 0;
        unsigned int k = 0;
        unsigned int j = 0;
        unsigned int freq = 0;
        unsigned int f = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        unsigned proceed = 0;
        FRAME *_frame = slot->frame_array;
        while(f < glob->Global_Frames->n){
        while(k < glob->Global_Sources->n){
                while(j < glob->Global_Destinations->n){
                        _frame = slot->frame_array;
                        unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
                        while(i < slot->n){
                                WiFi_Frame *frm = (WiFi_Frame*)_frame->frame_ptr;
                                if (frm->frame_type == (*(glob->Global_Frames->array+f))) proceed = 1;
                                if (proceed && (*(glob->Global_Sources->array+k) == frm->src_id) && (*(glob->Global_Destinations->array+j) == frm->dst_id)){
                                        val_array[freq] = frm->frame_length;
                                        freq++;
                                        avg += frm->frame_length;
                                }
                                _frame = _frame->next;
                                proceed = 0;
                                i++;
                        }
                        avg = _math_average(val_array, freq);
                        stdev = _math_stdev(_math_variance(val_array, avg, freq));
                        avg_dev = _math_avg_dev(val_array, freq);
                        if (isnan(avg_dev)) avg_dev = 0.0;
                        if (isnan(stdev)) stdev = 0.0;
                        unsigned int min = 0;
                        unsigned int max = 0;
                        _math_minmax(val_array, freq, &min, &max);
                        if (freq != 0)printf("%" PRIu64 ",%d,%d,%d,%d,%f,%f,%f,%d,%d\n", slot->slot_stop_time,
                                (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)), (*(glob->Global_Frames->array+f)), freq, avg, stdev,avg_dev, min, max);
                        freq = 0;
                        avg = 0.0;
                        i = 0;
                        free(val_array);
                        j++;
                }
                j = 0;
                k++;
        }
        k = 0; j = 0;
        f++;
        }
        glob->Global_Sources->n = 0;
        glob->Global_Destinations->n = 0;
        glob->Global_Frames->n = 0;
        i = 0;
}

void pro_ip_short(SLOT *slot, GLOBAL_KNOWLEDGE *glob){
        unsigned int i = 0;
        unsigned int k = 0;
        unsigned int j = 0;
        unsigned int freq = 0;
        unsigned int f = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        unsigned char proceed = 0;
        FRAME *_frame = slot->frame_array;
        while (f < glob->Global_Frames->n){
                while (k < glob->Global_Sources->n){   // for every source
                        while(j < glob->Global_Destinations->n){ // for every destination
                                _frame = slot->frame_array;
                                unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst$
                                while(i < slot->n){ // for each packet
                                        IP_Frame *frm = (IP_Frame*)_frame->frame_ptr;
                                        if (frm->protocol == (*(glob->Global_Frames->array+f))) proceed = 1;
                                        if (proceed && (*(glob->Global_Sources->array+k) == frm->src_ip) && (*(glob->Global_Destinations->array+j) == frm->dst_ip)){
                                                val_array[freq] = frm->packet_size;
                                                freq++;
                                                avg += frm->packet_size;
                                        }
                                        _frame = _frame->next;
                                        proceed = 0;
                                        i++;
                                }
                                avg = _math_average(val_array, freq);
                                stdev = _math_stdev(_math_variance(val_array, avg, freq));
                                avg_dev = _math_avg_dev(val_array, freq);
                                if (isnan(avg_dev)) avg_dev=0.0;
                                if (isnan(stdev)) stdev = 0.0;
                                unsigned int min = 0;
                                unsigned int max = 0;
                                _math_minmax(val_array, freq, &min, &max);
                                if (freq != 0)printf("%" PRIu64 ",%d,%d,%d,%d,%f,%f,%f,%d,%d\n", slot->slot_stop_time,
                                         (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)),(*(glob->Global_Frames->array+f)), freq, avg, stdev,avg_dev, min, max);
                                freq = 0;
                                avg = 0.0;
                                i = 0;
                                free(val_array);
                                j++;
                        }
                        j = 0;
                        k++;
                }
                k = 0; j = 0;
                f++;
        }
        glob->Global_Sources->n = 0;
        glob->Global_Destinations->n = 0;
        glob->Global_Frames->n = 0;
        i = 0;
}


void process_slot(SLOT *slot, GLOBAL_KNOWLEDGE *glob, unsigned char type){

	switch(type){
		case(ZBEE_TYPE):
				pro_zbee(slot, glob);
			break;
		case(WIFI_TYPE):
				pro_wifi(slot, glob);
			break;
		case(IPSH_TYPE):
				pro_ip_short(slot, glob);
			break;
		case(AUDI_TYPE):
				pro_audio(slot);
			break;
		default:

			break;
	}

}
