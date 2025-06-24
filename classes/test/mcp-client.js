{
  "mcpServers": {
    "filesystem": {
      "command": "node",
      "args": ["path/to/filesystem-server.js"],
      "env": {
        "ALLOWED_PATHS": "/Users/username/allowed-directory"
      }
    },
    "database": {
      "command": "python",
      "args": ["path/to/database-server.py", "--config", "db-config.json"],
      "env": {
        "DB_CONNECTION_STRING": "postgresql://user:pass@localhost/db"
      }
    },
    "web-search": {
      "command": "node",
      "args": ["path/to/web-search-server.js"],
      "env": {
        "API_KEY": "your-search-api-key"
      }
    }
  },
  "clientConfig": {
    "name": "my-mcp-client",
    "version": "1.0.0",
    "timeout": 30000,
    "retryAttempts": 3
  }
}


## 클라이언트 구현예
import { Client } from '@modelcontextprotocol/sdk/client/index.js';
import { StdioClientTransport } from '@modelcontextprotocol/sdk/client/stdio.js';

class MCPClient {
  private client: Client;
  private transport: StdioClientTransport;

  constructor(serverCommand: string, serverArgs: string[] = []) {
    // 서버와의 통신을 위한 transport 설정
    this.transport = new StdioClientTransport({
      command: serverCommand,
      args: serverArgs
    });

    // 클라이언트 초기화
    this.client = new Client({
      name: "mcp-demo-client",
      version: "1.0.0"
    }, {
      capabilities: {
        tools: {},
        resources: {},
        prompts: {}
      }
    });
  }

  async connect() {
    try {
      // 서버에 연결
      await this.client.connect(this.transport);
      
      // 초기화 및 capabilities 교환
      const serverInfo = await this.client.initialize();
      console.log('Connected to MCP server:', serverInfo);
      
      return serverInfo;
    } catch (error) {
      console.error('Failed to connect to MCP server:', error);
      throw error;
    }
  }

  async disconnect() {
    try {
      await this.client.close();
      console.log('Disconnected from MCP server');
    } catch (error) {
      console.error('Error disconnecting:', error);
    }
  }

  // 사용 가능한 도구 목록 조회
  async getAvailableTools() {
    try {
      const response = await this.client.listTools();
      return response.tools;
    } catch (error) {
      console.error('Failed to list tools:', error);
      throw error;
    }
  }

  // 도구 실행
  async executeTool(toolName: string, parameters: Record<string, any> = {}) {
    try {
      const result = await this.client.callTool({
        name: toolName,
        arguments: parameters
      });
      
      return result.content;
    } catch (error) {
      console.error(`Failed to execute tool ${toolName}:`, error);
      throw error;
    }
  }

  // 리소스 목록 조회
  async getAvailableResources() {
    try {
      const response = await this.client.listResources();
      return response.resources;
    } catch (error) {
      console.error('Failed to list resources:', error);
      throw error;
    }
  }

  // 리소스 읽기
  async readResource(uri: string) {
    try {
      const response = await this.client.readResource({ uri });
      return response.contents;
    } catch (error) {
      console.error(`Failed to read resource ${uri}:`, error);
      throw error;
    }
  }

  // 프롬프트 목록 조회
  async getAvailablePrompts() {
    try {
      const response = await this.client.listPrompts();
      return response.prompts;
    } catch (error) {
      console.error('Failed to list prompts:', error);
      throw error;
    }
  }

  // 프롬프트 실행
  async executePrompt(promptName: string, arguments: Record<string, any> = {}) {
    try {
      const result = await this.client.getPrompt({
        name: promptName,
        arguments: arguments
      });
      
      return result.messages;
    } catch (error) {
      console.error(`Failed to execute prompt ${promptName}:`, error);
      throw error;
    }
  }

  // 서버 상태 확인
  async ping() {
    try {
      await this.client.ping();
      return true;
    } catch (error) {
      console.error('Ping failed:', error);
      return false;
    }
  }
}

// 사용 예제
async function main() {
  const client = new MCPClient('node', ['path/to/your/mcp-server.js']);
  
  try {
    // 서버 연결
    await client.connect();
    
    // 사용 가능한 도구 확인
    const tools = await client.getAvailableTools();
    console.log('Available tools:', tools);
    
    // 도구 실행 예제
    if (tools.length > 0) {
      const result = await client.executeTool(tools[0].name, {
        // 도구에 필요한 매개변수
        param1: 'value1',
        param2: 'value2'
      });
      console.log('Tool execution result:', result);
    }
    
    // 리소스 확인
    const resources = await client.getAvailableResources();
    console.log('Available resources:', resources);
    
    // 리소스 읽기 예제
    if (resources.length > 0) {
      const resourceContent = await client.readResource(resources[0].uri);
      console.log('Resource content:', resourceContent);
    }
    
    // 프롬프트 확인 및 실행
    const prompts = await client.getAvailablePrompts();
    console.log('Available prompts:', prompts);
    
    if (prompts.length > 0) {
      const promptResult = await client.executePrompt(prompts[0].name, {
        input: 'Hello MCP!'
      });
      console.log('Prompt result:', promptResult);
    }
    
  } catch (error) {
    console.error('Error:', error);
  } finally {
    // 연결 종료
    await client.disconnect();
  }
}

// 에러 핸들링과 함께 실행
main().catch(console.error);

export default MCPClient;

## 파이션 mcp client
import asyncio
import json
import logging
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
import subprocess
import sys

# MCP 프로토콜 메시지 구조
@dataclass
class MCPMessage:
    jsonrpc: str = "2.0"
    id: Optional[str] = None
    method: Optional[str] = None
    params: Optional[Dict[str, Any]] = None
    result: Optional[Any] = None
    error: Optional[Dict[str, Any]] = None

class MCPClient:
    def __init__(self, server_command: List[str], client_name: str = "python-mcp-client"):
        self.server_command = server_command
        self.client_name = client_name
        self.client_version = "1.0.0"
        self.process = None
        self.message_id = 0
        self.logger = logging.getLogger(__name__)
        
    async def start(self):
        """MCP 서버 프로세스 시작"""
        try:
            self.process = await asyncio.create_subprocess_exec(
                *self.server_command,
                stdin=asyncio.subprocess.PIPE,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            self.logger.info(f"MCP 서버 시작됨: {' '.join(self.server_command)}")
            
            # 초기화 수행
            await self._initialize()
            
        except Exception as e:
            self.logger.error(f"서버 시작 실패: {e}")
            raise
    
    async def stop(self):
        """MCP 서버 프로세스 종료"""
        if self.process:
            self.process.terminate()
            await self.process.wait()
            self.logger.info("MCP 서버 종료됨")
    
    def _get_next_id(self) -> str:
        """다음 메시지 ID 생성"""
        self.message_id += 1
        return str(self.message_id)
    
    async def _send_message(self, message: MCPMessage) -> Dict[str, Any]:
        """서버에 메시지 전송 및 응답 수신"""
        if not self.process:
            raise RuntimeError("서버가 시작되지 않았습니다")
        
        # 메시지를 JSON으로 직렬화
        message_json = json.dumps({
            "jsonrpc": message.jsonrpc,
            "id": message.id,
            "method": message.method,
            "params": message.params or {}
        })
        
        self.logger.debug(f"전송: {message_json}")
        
        # 메시지 전송
        self.process.stdin.write((message_json + "\n").encode())
        await self.process.stdin.drain()
        
        # 응답 수신
        response_line = await self.process.stdout.readline()
        if not response_line:
            raise RuntimeError("서버로부터 응답을 받지 못했습니다")
        
        response_json = response_line.decode().strip()
        self.logger.debug(f"수신: {response_json}")
        
        try:
            response = json.loads(response_json)
            return response
        except json.JSONDecodeError as e:
            self.logger.error(f"JSON 파싱 오류: {e}")
            raise
    
    async def _initialize(self):
        """MCP 초기화 수행"""
        init_message = MCPMessage(
            id=self._get_next_id(),
            method="initialize",
            params={
                "protocolVersion": "2024-11-05",
                "capabilities": {
                    "tools": {},
                    "resources": {},
                    "prompts": {}
                },
                "clientInfo": {
                    "name": self.client_name,
                    "version": self.client_version
                }
            }
        )
        
        response = await self._send_message(init_message)
        
        if "error" in response:
            raise RuntimeError(f"초기화 실패: {response['error']}")
        
        self.logger.info("MCP 초기화 완료")
        return response.get("result", {})
    
    async def list_tools(self) -> List[Dict[str, Any]]:
        """사용 가능한 도구 목록 조회"""
        message = MCPMessage(
            id=self._get_next_id(),
            method="tools/list"
        )
        
        response = await self._send_message(message)
        
        if "error" in response:
            raise RuntimeError(f"도구 목록 조회 실패: {response['error']}")
        
        tools = response.get("result", {}).get("tools", [])
        self.logger.info(f"사용 가능한 도구 {len(tools)}개 발견")
        return tools
    
    async def call_tool(self, tool_name: str, arguments: Dict[str, Any] = None) -> Any:
        """도구 실행"""
        message = MCPMessage(
            id=self._get_next_id(),
            method="tools/call",
            params={
                "name": tool_name,
                "arguments": arguments or {}
            }
        )
        
        response = await self._send_message(message)
        
        if "error" in response:
            raise RuntimeError(f"도구 실행 실패: {response['error']}")
        
        result = response.get("result", {})
        self.logger.info(f"도구 '{tool_name}' 실행 완료")
        return result
    
    async def list_resources(self) -> List[Dict[str, Any]]:
        """사용 가능한 리소스 목록 조회"""
        message = MCPMessage(
            id=self._get_next_id(),
            method="resources/list"
        )
        
        response = await self._send_message(message)
        
        if "error" in response:
            raise RuntimeError(f"리소스 목록 조회 실패: {response['error']}")
        
        resources = response.get("result", {}).get("resources", [])
        self.logger.info(f"사용 가능한 리소스 {len(resources)}개 발견")
        return resources
    
    async def read_resource(self, uri: str) -> Any:
        """리소스 읽기"""
        message = MCPMessage(
            id=self._get_next_id(),
            method="resources/read",
            params={
                "uri": uri
            }
        )
        
        response = await self._send_message(message)
        
        if "error" in response:
            raise RuntimeError(f"리소스 읽기 실패: {response['error']}")
        
        result = response.get("result", {})
        self.logger.info(f"리소스 '{uri}' 읽기 완료")
        return result
    
    async def list_prompts(self) -> List[Dict[str, Any]]:
        """사용 가능한 프롬프트 목록 조회"""
        message = MCPMessage(
            id=self._get_next_id(),
            method="prompts/list"
        )
        
        response = await self._send_message(message)
        
        if "error" in response:
            raise RuntimeError(f"프롬프트 목록 조회 실패: {response['error']}")
        
        prompts = response.get("result", {}).get("prompts", [])
        self.logger.info(f"사용 가능한 프롬프트 {len(prompts)}개 발견")
        return prompts
    
    async def get_prompt(self, prompt_name: str, arguments: Dict[str, Any] = None) -> Any:
        """프롬프트 실행"""
        message = MCPMessage(
            id=self._get_next_id(),
            method="prompts/get",
            params={
                "name": prompt_name,
                "arguments": arguments or {}
            }
        )
        
        response = await self._send_message(message)
        
        if "error" in response:
            raise RuntimeError(f"프롬프트 실행 실패: {response['error']}")
        
        result = response.get("result", {})
        self.logger.info(f"프롬프트 '{prompt_name}' 실행 완료")
        return result

# 간단한 MCP 서버 시뮬레이터 (테스트용)
class SimpleMCPServer:
    """테스트용 간단한 MCP 서버"""
    
    def __init__(self):
        self.tools = [
            {
                "name": "echo",
                "description": "입력된 텍스트를 그대로 반환합니다",
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "text": {"type": "string", "description": "반환할 텍스트"}
                    },
                    "required": ["text"]
                }
            },
            {
                "name": "add",
                "description": "두 숫자를 더합니다",
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "a": {"type": "number", "description": "첫 번째 숫자"},
                        "b": {"type": "number", "description": "두 번째 숫자"}
                    },
                    "required": ["a", "b"]
                }
            }
        ]
    
    async def run(self):
        """서버 실행"""
        while True:
            try:
                line = input()
                if not line:
                    break
                
                request = json.loads(line)
                response = await self._handle_request(request)
                print(json.dumps(response))
                
            except EOFError:
                break
            except Exception as e:
                error_response = {
                    "jsonrpc": "2.0",
                    "id": request.get("id") if 'request' in locals() else None,
                    "error": {
                        "code": -32603,
                        "message": str(e)
                    }
                }
                print(json.dumps(error_response))
    
    async def _handle_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """요청 처리"""
        method = request.get("method")
        params = request.get("params", {})
        request_id = request.get("id")
        
        if method == "initialize":
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {
                        "tools": {"listChanged": True},
                        "resources": {},
                        "prompts": {}
                    },
                    "serverInfo": {
                        "name": "simple-mcp-server",
                        "version": "1.0.0"
                    }
                }
            }
        
        elif method == "tools/list":
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": {
                    "tools": self.tools
                }
            }
        
        elif method == "tools/call":
            tool_name = params.get("name")
            arguments = params.get("arguments", {})
            
            if tool_name == "echo":
                result = {"content": [{"type": "text", "text": arguments.get("text", "")}]}
            elif tool_name == "add":
                a = arguments.get("a", 0)
                b = arguments.get("b", 0)
                result = {"content": [{"type": "text", "text": str(a + b)}]}
            else:
                raise ValueError(f"알 수 없는 도구: {tool_name}")
            
            return {
                "jsonrpc": "2.0",
                "id": request_id,
                "result": result
            }
        
        else:
            raise ValueError(f"지원되지 않는 메서드: {method}")

# 사용 예제
async def main():
    """MCP 클라이언트 사용 예제"""
    # 로깅 설정
    logging.basicConfig(level=logging.INFO)
    
    # MCP 클라이언트 생성 (여기서는 테스트용 파이썬 서버 사용)
    client = MCPClient([sys.executable, "-c", """
import asyncio
import json

class SimpleMCPServer:
    def __init__(self):
        self.tools = [
            {"name": "echo", "description": "입력된 텍스트를 그대로 반환합니다",
             "inputSchema": {"type": "object", "properties": {"text": {"type": "string"}}, "required": ["text"]}},
            {"name": "add", "description": "두 숫자를 더합니다",
             "inputSchema": {"type": "object", "properties": {"a": {"type": "number"}, "b": {"type": "number"}}, "required": ["a", "b"]}}
        ]
    
    async def run(self):
        import sys
        while True:
            try:
                line = sys.stdin.readline()
                if not line:
                    break
                request = json.loads(line.strip())
                response = await self._handle_request(request)
                print(json.dumps(response), flush=True)
            except EOFError:
                break
            except Exception as e:
                error_response = {"jsonrpc": "2.0", "id": request.get("id") if 'request' in locals() else None, "error": {"code": -32603, "message": str(e)}}
                print(json.dumps(error_response), flush=True)
    
    async def _handle_request(self, request):
        method = request.get("method")
        params = request.get("params", {})
        request_id = request.get("id")
        
        if method == "initialize":
            return {"jsonrpc": "2.0", "id": request_id, "result": {"protocolVersion": "2024-11-05", "capabilities": {"tools": {"listChanged": True}}, "serverInfo": {"name": "simple-mcp-server", "version": "1.0.0"}}}
        elif method == "tools/list":
            return {"jsonrpc": "2.0", "id": request_id, "result": {"tools": self.tools}}
        elif method == "tools/call":
            tool_name = params.get("name")
            arguments = params.get("arguments", {})
            if tool_name == "echo":
                result = {"content": [{"type": "text", "text": arguments.get("text", "")}]}
            elif tool_name == "add":
                a = arguments.get("a", 0)
                b = arguments.get("b", 0)
                result = {"content": [{"type": "text", "text": str(a + b)}]}
            else:
                raise ValueError(f"알 수 없는 도구: {tool_name}")
            return {"jsonrpc": "2.0", "id": request_id, "result": result}
        else:
            raise ValueError(f"지원되지 않는 메서드: {method}")

server = SimpleMCPServer()
asyncio.run(server.run())
"""])
    
    try:
        # 서버 시작
        await client.start()
        
        # 사용 가능한 도구 목록 조회
        tools = await client.list_tools()
        print(f"사용 가능한 도구: {[tool['name'] for tool in tools]}")
        
        # echo 도구 실행
        echo_result = await client.call_tool("echo", {"text": "안녕하세요, MCP!"})
        print(f"Echo 결과: {echo_result}")
        
        # add 도구 실행
        add_result = await client.call_tool("add", {"a": 5, "b": 3})
        print(f"Add 결과: {add_result}")
        
    except Exception as e:
        print(f"오류 발생: {e}")
    
    finally:
        # 서버 종료
        await client.stop()

if __name__ == "__main__":
    asyncio.run(main())

## 예제
#!/usr/bin/env python3
"""
실용적인 MCP 클라이언트 예제
실제 MCP 서버(예: filesystem, database 등)와 연동
"""

import asyncio
import json
import logging
import sys
import os
from pathlib import Path
from typing import Dict, List, Any, Optional

class MCPClientManager:
    """여러 MCP 서버를 관리하는 클라이언트"""
    
    def __init__(self, config_file: Optional[str] = None):
        self.clients = {}
        self.config = self._load_config(config_file)
        self.logger = logging.getLogger(__name__)
    
    def _load_config(self, config_file: Optional[str]) -> Dict[str, Any]:
        """설정 파일 로드"""
        default_config = {
            "servers": {
                "filesystem": {
                    "command": ["python", "-m", "mcp_server_filesystem"],
                    "args": ["--base-path", str(Path.home())],
                    "enabled": True
                },
                "git": {
                    "command": ["python", "-m", "mcp_server_git"],
                    "args": ["--repository", "."],
                    "enabled": False
                }
            }
        }
        
        if config_file and os.path.exists(config_file):
            try:
                with open(config_file, 'r', encoding='utf-8') as f:
                    config = json.load(f)
                    return config
            except Exception as e:
                self.logger.warning(f"설정 파일 로드 실패, 기본값 사용: {e}")
        
        return default_config
    
    async def start_all_servers(self):
        """모든 활성화된 서버 시작"""
        for server_name, server_config in self.config["servers"].items():
            if server_config.get("enabled", True):
                try:
                    await self.start_server(server_name)
                except Exception as e:
                    self.logger.error(f"서버 {server_name} 시작 실패: {e}")
    
    async def start_server(self, server_name: str):
        """특정 서버 시작"""
        if server_name not in self.config["servers"]:
            raise ValueError(f"서버 {server_name}이 설정에 없습니다")
        
        server_config = self.config["servers"][server_name]
        command = server_config["command"] + server_config.get("args", [])
        
        client = MCPClient(command, f"mcp-manager-{server_name}")
        await client.start()
        self.clients[server_name] = client
        
        self.logger.info(f"서버 {server_name} 시작됨")
    
    async def stop_all_servers(self):
        """모든 서버 중지"""
        for server_name, client in self.clients.items():
            try:
                await client.stop()
                self.logger.info(f"서버 {server_name} 중지됨")
            except Exception as e:
                self.logger.error(f"서버 {server_name} 중지 실패: {e}")
        
        self.clients.clear()
    
    async def execute_command(self, server_name: str, command: str, **kwargs) -> Any:
        """특정 서버에서 명령 실행"""
        if server_name not in self.clients:
            raise ValueError(f"서버 {server_name}이 시작되지 않았습니다")
        
        client = self.clients[server_name]
        
        if command == "list_tools":
            return await client.list_tools()
        elif command == "call_tool":
            return await client.call_tool(kwargs.get("tool_name"), kwargs.get("arguments"))
        elif command == "list_resources":
            return await client.list_resources()
        elif command == "read_resource":
            return await client.read_resource(kwargs.get("uri"))
        else:
            raise ValueError(f"지원되지 않는 명령: {command}")
    
    def get_server_status(self) -> Dict[str, str]:
        """서버 상태 조회"""
        status = {}
        for server_name in self.config["servers"]:
            if server_name in self.clients:
                status[server_name] = "running"
            else:
                status[server_name] = "stopped"
        return status

class MCPClient:
    """기본 MCP 클라이언트 (이전 코드와 동일)"""
    
    def __init__(self, server_command: List[str], client_name: str = "python-mcp-client"):
        self.server_command = server_command
        self.client_name = client_name
        self.client_version = "1.0.0"
        self.process = None
        self.message_id = 0
        self.logger = logging.getLogger(__name__)
        
    async def start(self):
        """MCP 서버 프로세스 시작"""
        try:
            self.process = await asyncio.create_subprocess_exec(
                *self.server_command,
                stdin=asyncio.subprocess.PIPE,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            
            # 초기화 수행
            await self._initialize()
            
        except Exception as e:
            self.logger.error(f"서버 시작 실패: {e}")
            raise
    
    async def stop(self):
        """MCP 서버 프로세스 종료"""
        if self.process:
            self.process.terminate()
            await self.process.wait()
    
    def _get_next_id(self) -> str:
        """다음 메시지 ID 생성"""
        self.message_id += 1
        return str(self.message_id)
    
    async def _send_message(self, method: str, params: Dict[str, Any] = None) -> Dict[str, Any]:
        """서버에 메시지 전송 및 응답 수신"""
        if not self.process:
            raise RuntimeError("서버가 시작되지 않았습니다")
        
        message = {
            "jsonrpc": "2.0",
            "id": self._get_next_id(),
            "method": method,
            "params": params or {}
        }
        
        message_json = json.dumps(message)
        self.process.stdin.write((message_json + "\n").encode())
        await self.process.stdin.drain()
        
        response_line = await self.process.stdout.readline()
        if not response_line:
            raise RuntimeError("서버로부터 응답을 받지 못했습니다")
        
        response = json.loads(response_line.decode().strip())
        
        if "error" in response:
            raise RuntimeError(f"서버 오류: {response['error']}")
        
        return response.get("result", {})
    
    async def _initialize(self):
        """MCP 초기화 수행"""
        result = await self._send_message("initialize", {
            "protocolVersion": "2024-11-05",
            "capabilities": {
                "tools": {},
                "resources": {},
                "prompts": {}
            },
            "clientInfo": {
                "name": self.client_name,
                "version": self.client_version
            }
        })
        
        self.logger.info("MCP 초기화 완료")
        return result
    
    async def list_tools(self) -> List[Dict[str, Any]]:
        """사용 가능한 도구 목록 조회"""
        result = await self._send_message("tools/list")
        return result.get("tools", [])
    
    async def call_tool(self, tool_name: str, arguments: Dict[str, Any] = None) -> Any:
        """도구 실행"""
        result = await self._send_message("tools/call", {
            "name": tool_name,
            "arguments": arguments or {}
        })
        return result
    
    async def list_resources(self) -> List[Dict[str, Any]]:
        """사용 가능한 리소스 목록 조회"""
        result = await self._send_message("resources/list")
        return result.get("resources", [])
    
    async def read_resource(self, uri: str) -> Any:
        """리소스 읽기"""
        result = await self._send_message("resources/read", {"uri": uri})
        return result

# 대화형 클라이언트
class InteractiveMCPClient:
    """대화형 MCP 클라이언트"""
    
    def __init__(self):
        self.manager = MCPClientManager()
        self.logger = logging.getLogger(__name__)
    
    async def run(self):
        """대화형 클라이언트 실행"""
        print("=== MCP 클라이언트 시작 ===")
        print("사용 가능한 명령:")
        print("  start <서버명> - 서버 시작")
        print("  stop <서버명> - 서버 중지")
        print("  status - 서버 상태 조회")
        print("  tools <서버명> - 도구 목록 조회")
        print("  call <서버명> <도구명> [인수] - 도구 실행")
        print("  resources <서버명> - 리소스 목록 조회")
        print("  read <서버명> <URI> - 리소스 읽기")
        print("  quit - 종료")
        print()
        
        try:
            while True:
                try:
                    command = input("MCP> ").strip().split()
                    if not command:
                        continue
                    
                    await self._handle_command(command)
                    
                except KeyboardInterrupt:
                    print("\n종료 중...")
                    break
                except EOFError:
                    break
                except Exception as e:
                    print(f"오류: {e}")
        
        finally:
            await self.manager.stop_all_servers()
    
    async def _handle_command(self, command: List[str]):
        """명령 처리"""
        cmd = command[0].lower()
        
        if cmd == "quit" or cmd == "exit":
            raise KeyboardInterrupt
        
        elif cmd == "start":
            if len(command) < 2:
                print("사용법: start <서버명>")
                return
            
            server_name = command[1]
            await self.manager.start_server(server_name)
            print(f"서버 {server_name} 시작됨")
        
        elif cmd == "stop":
            if len(command) < 2:
                print("사용법: stop <서버명>")
                return
            
            server_name = command[1]
            if server_name in self.manager.clients:
                await self.manager.clients[server_name].stop()
                del self.manager.clients[server_name]
                print(f"서버 {server_name} 중지됨")
            else:
                print(f"서버 {server_name}이 실행 중이 아닙니다")
        
        elif cmd == "status":
            status = self.manager.get_server_status()
            print("서버 상태:")
            for server_name, server_status in status.items():
                print(f"  {server_name}: {server_status}")
        
        elif cmd == "tools":
            if len(command) < 2:
                print("사용법: tools <서버명>")
                return
            
            server_name = command[1]
            tools = await self.manager.execute_command(server_name, "list_tools")
            print(f"{server_name} 서버의 도구:")
            for tool in tools:
                print(f"  - {tool['name']}: {tool.get('description', '')}")
        
        elif cmd == "call":
            if len(command) < 3:
                print("사용법: call <서버명> <도구명> [JSON 인수]")
                return
            
            server_name = command[1]
            tool_name = command[2]
            arguments = {}
            
            if len(command) > 3:
                try:
                    arguments = json.loads(" ".join(command[3:]))
                except json.JSONDecodeError:
                    print("인수는 유효한 JSON 형식이어야 합니다")
                    return
            
            result = await self.manager.execute_command(
                server_name, "call_tool", 
                tool_name=tool_name, arguments=arguments
            )
            print(f"결과: {json.dumps(result, indent=2, ensure_ascii=False)}")
        
        elif cmd == "resources":
            if len(command) < 2:
                print("사용법: resources <서버명>")
                return
            
            server_name = command[1]
            resources = await self.manager.execute_command(server_name, "list_resources")
            print(f"{server_name} 서버의 리소스:")
            for resource in resources:
                print(f"  - {resource['uri']}: {resource.get('

