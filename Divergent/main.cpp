#include<Windows.h>
#include<gl/GL.h>
#include<gl/glext.h>

#define WIDTH 1920;
#define HEIGHT 1080;
#define INSTANCE 0x400000;


static unsigned int startTime;

static unsigned int midiOut;
static unsigned int curNote = 0;
static unsigned int MTIMER = 1024;

static unsigned int mulCurNote = 134217728 / 2;

static unsigned char music[] = { 0x30,0x38,0x35,0x3F,0x33,0x3F,0x30,0x3B,
                                       0x2B,0x20,0x2F,0x23,0x2F,0x25,0x28,0x20,
	                                   0x10,0x18,0x15,0x1F,0x13,0x1F,0x10,0x1B,
                                       0x0B,0x00,0x0F,0x03,0x0F,0x05,0x08,0x00, };

static PIXELFORMATDESCRIPTOR pfd = {0,0,37,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static char* glUseProgramStr = "glUseProgram";
static char* glCreateShaderProgramvStr = "glCreateShaderProgramv";
/*
const static char* fragmentSource = "\
\
float t=gl_Color.r*1000;\
void main()\
{\
    vec2 r=(vec2(400.0f,600.0f)+gl_FragCoord.xy)/500;\
	gl_FragColor = vec4(mod(abs(sin(5-r.x*50*cos(t))*cos(5-r.y*50*sin(t))),abs(abs(cos(t))*0.36+0.24*abs(sin(t)))),mod(abs(sin(r.x*100*sin(t))*cos(r.y*100*cos(t))),abs(abs(cos(t))*0.76+0.24*abs(sin(t)))),sin(4*t+20*(r.x*r.x+r.y*r.y)),1.0f);\
}";*/

static char* fragmentSource =
"vec3 v=vec3(0,0,20),r=v,y,c=vec3(gl_FragCoord-vec2(960,540),-2607.35);"
"float s=256000,n=.0001,z=gl_Color.z*s,x=gl_Color.x,m=gl_Color.y,e=c.x*c.x+c.y*c.y,i,g;"
"float t(vec3 v)"
"{"
"v=vec3(v.x*cos(z)+v.z*sin(z),v.y,-v.x*sin(z)+v.z*cos(z));"
"if(m==0)"
"v=mod(v,vec3(32))-vec3(16);"
"return sin(z*v.x)-.7*cos(z*v.y)+.5*sin(z*v.z)+length(max(abs(v)-vec3(1),0));"
"}"
"vec3 f(vec3 z)"
"{"
"return y=normalize(vec3(t(r+vec3(n,0,0))-t(r-vec3(n,0,0)),t(r+vec3(0,n,0))-t(r-vec3(0,n,0)),t(r+vec3(0,0,n))-t(r-vec3(0,0,n)))),z=normalize(z-r),.4*(vec3(.7-x,.2+x,.2*x)*max(dot(z,y),0)+pow(max(dot(normalize(reflect(-z,y)),normalize(v-r)),0),10));"
"}"
"void main()"
"{"
"for(float v=255;v;v--)"
"{"
"g=t(r);"
"r+=g*normalize(c);"
"i+=g;"
"gl_FragColor=g<n?vec4(vec3(.1)+f(vec3(4*sin(z),2,4*cos(z)))+f(vec3(2*sin(.37*z),2*cos(.37*z),2)),1):e>s*2*x&&e<s*4*x?vec4(m?x:1-x):vec4(m?1-x:x);"
"if(g<n)"
"return;"
"}"
"}";


int WINAPI WinMainCRTStartup()
{
	//Setup Window
	//wc.cbSize = 48;//sizeof(WNDCLASSEX);
	//80 Memory
	__asm
	{
		push 0
		push 0
		push 0
		push 0
		push HEIGHT
		push WIDTH
		push 0
		push 0
		push 0x91000000
		push 0
		push 0x0C018
		push 0
		//push esi


		//call RegisterClassExA
		call CreateWindowExA
		push eax
		call GetDC
		mov edi,eax
		lea ecx, pfd
		lea ebx, midiOut
		push edi
		push ecx
		push edi
		call ChoosePixelFormat
		pop edi
		push edi
		push ebx
		push eax
		push edi
		call SetPixelFormat
		pop edi
		push edi
		push edi
		call wglCreateContext
		pop edi
		push edi
		push eax
		push edi
		call wglMakeCurrent


		mov eax, glCreateShaderProgramvStr
		push edi
		push eax
		call wglGetProcAddress
		pop edi
		push edi

		lea ecx, fragmentSource
		push edi
		push ecx
		push 1
		push GL_FRAGMENT_SHADER
		call eax
		pop edi
		push edi
		push eax

		mov eax, glUseProgramStr
		pop edi
		push edi
		push eax
		call wglGetProcAddress
		pop edi
		push edi
		call eax
		pop edi

		push 0
		push 0
		push 0
		push 0
		push 0
		push ebx
		call midiOutOpen
		call ShowCursor


/*		push 0x7EC0
		push ebx
		call midiOutShortMsg*/



		call timeGetTime
		//sub eax,700*60*1
		mov startTime, eax

		xor esi,esi
		//xor edx,edx
begin:

		call timeGetTime
		mov edx, [midiOut]
		mov ecx, startTime
		sub eax,ecx
		//cmp eax,700*60+1000*93
		push 0x1B
		push 1
		push 0
		push 0
		push 0
		push 0
		push edi
		push - 1
		push - 1
		push 1
		push 1
		push eax
		//jge end


		and eax,MTIMER
		xor eax,esi
		jnz draw
		mov ebx,curNote
		mov al, [music + ebx]
		xor esi,MTIMER
		jnz play_note

		inc ebx
		and ebx, 31
		mov curNote, ebx
		jnz old_speed
	    shr MTIMER,1
		mov ecx, MTIMER
		cmp ecx, 32
		jz end
old_speed:
		neg ebx
		add ebx,31
		mov al, [music + ebx]
		neg ebx
		add ebx,31

play_note:
		shl eax, 8
		or eax, 0x007F0090

		push eax
		push edx
		call midiOutShortMsg
draw:
		mov eax, ebx
		shl eax,27
		push esi
		push eax
		call glColor3ui
		call glRects
		//push 1
		call SwapBuffers
		call PeekMessageA
		call GetAsyncKeyState
		cmp eax, 0
		je short begin
end:
		call ExitProcess
	}
}