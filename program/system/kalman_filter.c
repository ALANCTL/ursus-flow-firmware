/* process error covariance matrix */
float q11 = 0.65f, q12 = 0.0f;
float q21 = 0.0f,  q22 = 0.65f;

/* measurement error covariance matrix */
float r11 = 1.0f, r12 = 0.0f;
float r21 = 0.0f, r22 = 1.0f;

/* kalman gain matrix */
float g11 = 0.0f, g12 = 0.0f;
float g21 = 0.0f, g22 = 0.0f;

float p11_last = 0.0f, p12_last = 0.0f;
float p21_last = 0.0f, p22_last = 0.0f;

float p11_now = 0.0f, p12_now = 0.0f;
float p21_now = 0.0f, p22_now = 0.0f;

float kalman_vx_last = 0.0f;
float kalman_vy_last = 0.0f;

float vx_bias = -1.5;
float vy_bias = -0.2;

__attribute__((section(".itcmtext")))
void kalman_filter(float *kalman_vx, float *kalman_vy, float flow_vx, float flow_vy,
                   float accel_ax, float accel_ay, float delta_t)
{
	/* predict */
	float vx_predict = kalman_vx_last + (accel_ax * delta_t);
	float vy_predict = kalman_vy_last + (accel_ay * delta_t);

	p11_now = p11_last + q11;
	p22_now = p22_last + q22;

	/* update */
	g11 = (p11_now) / (p11_now + r11); //gain
	g22 = (p22_now) / (p22_now + r22);

	*kalman_vx = vx_predict + g11 * (flow_vx - vx_predict);
	*kalman_vy = vy_predict + g22 * (flow_vy - vy_predict);

	p11_last = (1.0f - g11) * p11_now;
	p22_last = (1.0f - g22) * p22_now;

	/* save for next iteration */
	kalman_vx_last = *kalman_vx;
	kalman_vy_last = *kalman_vy;

	/* bias correction */
	*kalman_vx -= vx_bias;
	*kalman_vy -= vy_bias;
}
