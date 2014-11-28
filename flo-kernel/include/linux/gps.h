/**
 * Header file of GPS location
 */

/* sent from user space */
struct gps_location {
	double latitude;
	double longitude;
	float accuracy;	/* In meter */
};

struct gps_kdata {
	unsigned char m_lat[8];	/* represent 64 bit double 	*/
	unsigned char m_lon[8];	/* represent 64 bit double 	*/	
	unsigned char m_acc[4];	/* represent 32 bit float 	*/
	unsigned char m_age[4];	/* represent 32 bit integer	*/
};
