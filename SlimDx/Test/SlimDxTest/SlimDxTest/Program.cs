using SlimDX;
using SlimDX.D3DCompiler;
using SlimDX.Direct3D11;
using SlimDX.DXGI;
using SlimDX.Windows;
using System.Drawing;
using System.Windows.Forms;
using Buffer = SlimDX.Direct3D11.Buffer;
using Device = SlimDX.Direct3D11.Device;
using Resource = SlimDX.Direct3D11.Resource;

namespace SlimDxTest
{
	public class Program
	{
		private static DeviceContext _context;
		private static Device _device;
		private static SwapChain _swap;
		private static Result _result;
		private static RenderTargetView _render;

		public static void Main()
		{
			var form = new RenderForm
			{
				ClientSize = new Size
				{
					Width = 1920,
					Height = 1080,
				},
				WindowState = FormWindowState.Maximized,
			};
			Setup(form);
			MessagePump.Run(form, () =>
			{
				// clear the render target to a soothing blue
				_context.ClearRenderTargetView(_render, new Color4(0.5f, 0.5f, 1.0f));
				_context.Draw(16, 0);
				_swap.Present(0, PresentFlags.None);
			});
		}

		private static void Setup(RenderForm form)
		{
			// create description for the swap chain
			var description = new SwapChainDescription
			{
				BufferCount = 1,
				Usage = Usage.RenderTargetOutput,
				OutputHandle = form.Handle,
				IsWindowed = true,
				ModeDescription = new ModeDescription(0, 0, new Rational(60, 1), Format.R8G8B8A8_UNorm),
				SampleDescription = new SampleDescription(1, 0),
				Flags = SwapChainFlags.AllowModeSwitch,
				SwapEffect = SwapEffect.Discard
			};
			// create DirectX3D device with swap chain
			_result = Device.CreateWithSwapChain(
				DriverType.Hardware,
				DeviceCreationFlags.None,
				new[] { Device.GetSupportedFeatureLevel() },
				description,
				out _device,
				out _swap);
			// disable default alt enter key behavior (toggle full-screen mode)
			using (var factory = _swap.GetParent<Factory>())
				factory.SetWindowAssociation(form.Handle, WindowAssociationFlags.IgnoreAltEnter);
			// set key down event including new alt enter key behaviour (toggle full-screen mode)
			form.KeyDown += (o, e) =>
			{
				if (e.KeyCode == Keys.Escape) Application.Exit();
				if (e.Alt && e.KeyCode == Keys.Enter) _swap.IsFullScreen = !_swap.IsFullScreen;
			};
			// get default device context, default to primary display if there is multi-screens
			_context = _device.ImmediateContext;
			// create the view port for the device context and set view port to device context
			_context.Rasterizer.SetViewports(new Viewport(0.0f, 0.0f, form.ClientSize.Width, form.ClientSize.Height));
			// get render view from swap chain
			using (var resource = Resource.FromSwapChain<Texture2D>(_swap, 0))
				_render = new RenderTargetView(_device, resource);
			// set render view to device context
			_context.OutputMerger.SetTargets(_render);
			// set application exit action with disposing all slimdx objects
			Application.ApplicationExit += (sender, eventArgs) =>
			{
				//TESTED AND WORKING: dispose of objects.
				var objects = ObjectTable.Objects;
				foreach (var obj in objects)
				{
					obj.Dispose();
				}
				if (_context != null)
				{
					_context.Dispose();
					_context = null;
				}
				if (_device != null)
				{
					_device.Dispose();
					_device = null;
				}
				if (_swap != null)
				{
					_swap.Dispose();
					_swap = null;
				}
				if (_render != null)
				{
					_render.Dispose();
					_render = null;
				}
			};
			// create new vertices
			const int sizeInBytes = 12 * 3;
			var vertices = new DataStream(sizeInBytes, true, true);
			vertices.Write(new Vector3(0.0f, 0.5f, 1f));
			vertices.Write(new Vector3(0.5f, -0.5f, 1f));
			vertices.Write(new Vector3(-0.5f, -0.5f, 1f));
			vertices.Position = 0;
			var vertexBuffer = new Buffer(_device, vertices, sizeInBytes, ResourceUsage.Default, BindFlags.VertexBuffer, CpuAccessFlags.None, ResourceOptionFlags.None, 0);

			using (var vertex_bytecode = ShaderBytecode.CompileFromFile("shader.fx", "VShader", "vs_4_0", ShaderFlags.None, EffectFlags.None))
			using (var pixel_bytecode = ShaderBytecode.CompileFromFile("shader.fx", "PShader", "ps_4_0", ShaderFlags.None, EffectFlags.None))
			{
				// load and compile the pixel shader
				var signature = ShaderSignature.GetInputSignature(pixel_bytecode);
				var elements = new[] { new InputElement("SV_POSITION", 0, Format.R32G32B32_Float, 0) };

				_context.InputAssembler.InputLayout = new InputLayout(_device, signature, elements);
				_context.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
				_context.InputAssembler.SetVertexBuffers(0, new VertexBufferBinding(vertexBuffer, 12, 0));
				_context.VertexShader.Set(new VertexShader(_device, vertex_bytecode));
				_context.PixelShader.Set(new PixelShader(_device, pixel_bytecode));
			}

			//swapChain.IsFullScreen = true;// default to full-screen

		}
	}
}
