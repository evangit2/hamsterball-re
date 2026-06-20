
int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)

{
  App_Initialize_Full(&g_App,hInstance,lpCmdLine);
  App_Run((int *)&g_App);
  App_Shutdown((int *)&g_App);
  return 0;
}

