#include "maxcpp5.h"
#include "ext_obex.h"

#include "vrpn/vrpn_Connection.h"
#include "vrpn/vrpn_Tracker.h"

static t_symbol * ps_quat;
static t_symbol * ps_position;
static t_symbol * ps_sensor;

// for redirecting messages, see vrpn_TextPrinter and add_object method
// need to figure out how to create a FILE pointer to use with post() etc.

class VRPNTracker : public MaxCpp5<VRPNTracker> {
public:
	vrpn_Tracker_Remote * tkr;
	vrpn_TRACKERCB data;
	char name[4096];
	
	float scale[3];
	long scale_count;
	
	static void tracker_handler(void * u, const vrpn_TRACKERCB d) {
		VRPNTracker * tracker = (VRPNTracker *)u;
		memcpy(&tracker->data, &d, sizeof(vrpn_TRACKERCB));
	}
	
//	static t_max_err getscale(VRPNTracker *x, void *attr, long *ac, t_atom **av);
//	static t_max_err setscale(VRPNTracker *x, void *attr, long ac, t_atom *av);
	
	
	VRPNTracker(t_symbol * sym, long ac, t_atom * av) { 
		setupIO(1, 1); // inlets / outlets
		
		scale_count = 3;
		scale[0] = 1.;
		scale[1] = 1.;
		scale[2] = -1.;
		
		// get address:
		const char * body = ac > 0 && av[0].a_type == A_SYM ? atom_getsym(av+0)->s_name : "RigidBody1";
		const char * ip = ac > 1 && av[1].a_type == A_SYM ? atom_getsym(av+1)->s_name : "127.0.0.1";
		int port = ac > 2 ? atom_getlong(av+2) : 3883;
		
		//attr_args_process(this, ac, av);
		
		sprintf(name, "%s@%s:%d", body, ip, port);
		object_post(&m_ob, "created vrpn tracker %s", name);
		
		tkr = new vrpn_Tracker_Remote(name);	
		tkr->register_change_handler((void *)this, &tracker_handler);
		
		
	}
	~VRPNTracker() {
		delete tkr;
	}	
	
	// methods:
	void bang(long inlet) { 
		t_atom list[4];
		tkr->mainloop();
		
		// convert quaterion to Cosm layout
		atom_setfloat(list+0, data.quat[3]);
		atom_setfloat(list+1, data.quat[0] * -1.);
		atom_setfloat(list+2, data.quat[1] * -1.);
		atom_setfloat(list+3, data.quat[2]);
		outlet_anything(m_outlet[0], ps_quat, 4, list);
		
		atom_setfloat(list+0, data.pos[0] * scale[0]);
		atom_setfloat(list+1, data.pos[1] * scale[1]);
		atom_setfloat(list+2, data.pos[2] * scale[2]);
		outlet_anything(m_outlet[0], ps_position, 3, list);
		
		atom_setlong(list, data.sensor);
		outlet_anything(m_outlet[0], ps_sensor, 1, list);
	}
};

//t_max_err VRPNTracker :: getscale(VRPNTracker *x, void *attr, long *ac, t_atom **av)
//{
//	*av = (t_atom *)getbytes(sizeof(t_atom)*3);
//	if (!*av) {
//		*ac = 0;
//		return MAX_ERR_OUT_OF_MEM;
//	}
//	*ac = 3;
//	
//	atom_setfloat(*av, x->scale[0]);
//	atom_setfloat((*av)+1, x->scale[1]);
//	atom_setfloat((*av)+2, x->scale[2]);
//	return MAX_ERR_NONE;
//}
//
//t_max_err VRPNTracker :: setscale(VRPNTracker *x, void *attr, long ac, t_atom *av)
//{
//	if (ac > 0 && av) 
//	{
//		x->scale[0] = atom_getfloat(av);
//		x->scale[1] = atom_getfloat(ac > 1 ? av+1 : av); 
//		x->scale[2] = atom_getfloat(ac > 2 ? av+2 : av);
//	}
//	return MAX_ERR_NONE;
//}

extern "C" int main(void) {
	ps_quat = gensym("quat");
	ps_position = gensym("position");
	ps_sensor = gensym("sensor");

	// create a class with the given name:
	t_class * c = VRPNTracker::makeMaxClass("vrpn.tracker");
	REGISTER_METHOD(VRPNTracker, bang);
	
//	// add attributes:
//	class_addattr(c, (t_object *)attr_offset_array_new(
//		"scale", 
//		_sym_float, 
//		3, 
//		0,
//		(method)0L, //VRPNTracker :: getscale, 
//		(method)VRPNTracker :: setscale, 
//		0L,
//		0L
//	));
}